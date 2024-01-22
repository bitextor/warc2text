
#include <iostream>
#include "src/bilangwriter.hh"
#include "warcpreprocessor.hh"
#include "src/lang.hh"
#include "zipreader.hh"
#include "util/compress.hh"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace {
    const std::string kRobotsTxtPath = "/robots.txt";

    bool isRobotsTxt(const warc2text::Record &record) {
        const auto &url = record.getURL();

        // Find the bit after https://
        auto host_offset = url.find("://");
        if (host_offset != std::string::npos) {
            host_offset += 3; // len(://)
        }
        // maybe it is a relative url, i.e. //hostname?
        else if (url.substr(0, 2) == "//") {
            host_offset = 2; // len(//)
        }
        // Just assume there is no protocol, and we start with the hostname.
        else {
            host_offset = 0;
        }

        // Find the beginning of the path
        auto path_offset = url.find("/", host_offset);
        if (path_offset == std::string::npos)
            return false;

        // If the first bit of the path is robots.txt, that's hopeful.
        if (url.compare(path_offset, kRobotsTxtPath.size(), kRobotsTxtPath) != 0)
            return false;

        // Is there anything after the /robots.txt?
        if (url.size() > path_offset + kRobotsTxtPath.size())
            return false;

        return true;
    }

    bool isPDF(const warc2text::Record &record) {
        if (record.isTextFormat())
            return false;

        // if HTTP content type is 'text/html' or something similar, don't rely
        // on URL extension to detect unprocessed PDFs. PDFs that have gone
        // through bitextor-warc2htmlwarc.py will have URL ending in .pdf but
        // text HTTP content type.
        if (boost::algorithm::ends_with(record.getURL(), ".pdf"))
            return true;

        if (record.getHTTPcontentType() == "application/pdf")
            return true;

        return false;
    }
}

namespace warc2text {
    const std::unordered_set<std::string> WARCPreprocessor::removeExtensions = {".jpg", ".jpeg", ".gif", ".png", ".css", ".js", ".mp3",
                                                                                ".mp4", ".flv", ".wmv", ".gz", ".zip", ".rar" };

    WARCPreprocessor::WARCPreprocessor(RecordWriter &writer, const LanguageDetector &detector, WARCPreprocessorOptions const &options) :
        writer(writer),
        detector(detector),
        options(options),
        totalRecords(0),
        textRecords(0),
        langRecords(0),
        totalBytes(0),
        textBytes(0),
        langBytes(0),
        tagFilters() {
            if (!options.tag_filters_filename.empty())
                util::readTagFiltersRegex(options.tag_filters_filename, tagFilters);

            if (!options.url_filters_filename.empty())
                util::readUrlFiltersRegex(options.url_filters_filename, urlFilter);

            if (!options.pdf_warc_filename.empty())
                pdf_warc_writer.open(options.pdf_warc_filename);

            if (!options.robots_warc_filename.empty())
                robots_warc_writer.open(options.robots_warc_filename);
        }

    // true if url is good
    bool WARCPreprocessor::URLfilter(const std::string& url) const {
        for (const std::string& ext : removeExtensions)
            if (boost::algorithm::ends_with(url, ext))
                return false;

        if (!urlFilter.empty() && boost::regex_search(url, urlFilter)) {
            BOOST_LOG_TRIVIAL(info) << "Url filter matched '" << url << "'";
            return false;
        }
        
        return true;
    }

    void WARCPreprocessor::process(const std::string& filename) {
        BOOST_LOG_TRIVIAL(info) << "Processing " << filename;
        WARCReader reader(filename);

        std::string content;
        int n_langs = 0;

        while (true) {
            std::size_t offset = reader.tell();
            std::size_t size = reader.getRecord(content);
            
            // No more records (EOF or failure to inflate)
            if (size == 0)
                break;

            // Note that content.empty() will also be true when len(record) > max_size (which is 20MB by default)
            if (content.empty())
                continue;

            Record record(content, filename, size, offset);
            if (record.getPayload().empty())
                continue;

            // Pick out all robots.txt related records.
            if (::isRobotsTxt(record)) {
                robots_warc_writer.writeRecord(content); // no-op if robots_warc_writer is not opened.
                continue;
            }

            if (record.getRecordType() != "response" && record.getRecordType() != "resource")
                continue;

            if (record.getWARCcontentType().find("application/http") == std::string::npos)
                continue;

            if (::isPDF(record)) {
                // found a PDF file, write record to disk and continue
                // this is a no-op if pdf_warc_writer is not opened.
                pdf_warc_writer.writeRecord(content);
                continue;
            }

            if (record.getPayload().size() > 5242880) // 5MB
                continue;

            if (!URLfilter(record.getURL()))
                continue;

            if (options.encodeURLs)
                record.encodeURL();

            BOOST_LOG_TRIVIAL(trace) << "Processing HTML document " << record.getURL() << "\n";

            ++totalRecords;
            totalBytes += record.getPayload().size();

            int clean_retval;
            try{
                clean_retval = record.cleanPayload(tagFilters, options.skip_text_extraction);
            }
            catch (std::out_of_range& e) { continue; }
            catch (std::invalid_argument& e) { continue; }
            catch (util::ZipReadError& e) {
                BOOST_LOG_TRIVIAL(info) << "Record " << record.getURL() << " discarded due to invalid zip file: " << e.what();
                continue;
            }

            if ((clean_retval == util::FILTERED_DOCUMENT_ERROR) != options.tag_filters_invert) {
                BOOST_LOG_TRIVIAL(info) << "Record " << record.getURL() << " discarded due to tag filters";
                continue;
            } else if (clean_retval == util::HTML_PARSING_ERROR) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": parsing error";
                continue;
            } else if (clean_retval == util::UNKNOWN_ENCODING_ERROR) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": unknown encoding";
                continue;
            } else if (clean_retval == util::UTF8_CONVERSION_ERROR) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": utf8 conversion error";
                continue;
            } else if (clean_retval == util::NOT_VALID_RECORD) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": WARC or HTTP header content type not valid";
                continue;
            }

            if (record.getPlainText().empty() && !options.skip_text_extraction) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": empty";
                continue;
            }

            ++textRecords;
            // When skipping text extraction sum payload bytes because text is empty
            if (options.skip_text_extraction)
                textBytes += record.getPayload().size();
            else
                textBytes += record.getPlainText().size();

            record.detectLanguage(detector);
            n_langs = 0;
            for (auto const &chunk : record.getTextByLangs()) {
                // Don't count the unknown language chunks
                if (chunk.first == LanguageDetector::kUnknownLanguageLabel)
                    continue;
                
                langBytes += chunk.second.size();
                ++n_langs;
            }

            if (n_langs > 1) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": multiple (" << n_langs << ") languages detected";
            } else if (n_langs == 1) {

            } else {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": language not detected";
            }

            langRecords += n_langs;

            writer.write(record, options.paragraph_identification);
        }
    }

    void WARCPreprocessor::printStatistics() const{
        BOOST_LOG_TRIVIAL(info) << "total records: " << totalRecords;
        if (options.skip_text_extraction) {
            BOOST_LOG_TRIVIAL(info) << "extracted records: " << textRecords;
        } else {
            BOOST_LOG_TRIVIAL(info) << "text records: " << textRecords;
            BOOST_LOG_TRIVIAL(info) << "lang records: " << langRecords;
        }

        BOOST_LOG_TRIVIAL(info) << "total bytes: " << totalBytes;
        if (options.skip_text_extraction) {
            BOOST_LOG_TRIVIAL(info) << "extracted bytes: " << textBytes;
        } else {
            BOOST_LOG_TRIVIAL(info) << "text bytes: " << textBytes;
            BOOST_LOG_TRIVIAL(info) << "lang bytes: " << langBytes;
        }
    }

    WARCWriter::WARCWriter() {
        warc = nullptr;
    }

    WARCWriter::~WARCWriter() {
        close();
    }

    void WARCWriter::open(const std::string& warc_filename) {
        filename = warc_filename;
        if (not boost::algorithm::ends_with(filename, ".warc.gz"))
            filename += ".warc.gz";
        auto filename_offset = filename.find_last_of('/');
        if (filename_offset != std::string::npos) {
            std::string folder = filename.substr(0, filename_offset);
            util::createDirectories(folder);
        }
        warc = std::fopen(filename.c_str(), "wb");
    }

    bool WARCWriter::is_open() {
        return warc != nullptr;
    }

    void WARCWriter::close() {
        if (warc) std::fclose(warc);
    }

    void WARCWriter::writeRecord(const std::string& content) {
        if (!warc) return;
        std::string compressed;
        util::GZCompress(content, compressed);
        std::fwrite((void*) compressed.c_str(), 1, compressed.size(), warc);
    }

}
