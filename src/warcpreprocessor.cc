#include "warcpreprocessor.hh"
#include "util/compress.hh"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace warc2text {
    const std::unordered_set<std::string> WARCPreprocessor::textContentTypes = {"text/plain", "text/html", "application/xml"};
    const std::unordered_set<std::string> WARCPreprocessor::removeExtensions = {".jpg", ".jpeg", ".gif", ".png", ".css", ".js", ".mp3", ".mp4", ".flv", ".wmv", ".gz", ".zip", ".rar" };

    WARCPreprocessor::WARCPreprocessor(const std::string& outputFolder, const std::unordered_set<std::string>& output_files, const std::string pdf_warc_filename, const std::string& tagFiltersFile) :
        writer(outputFolder, output_files),
        totalRecords(0),
        textRecords(0),
        langRecords(0),
        totalBytes(0),
        textBytes(0),
        langBytes(0),
        tagFilters(),
        pdf_warc_filename(pdf_warc_filename) {
            if (!tagFiltersFile.empty())
                util::readTagFilters(tagFiltersFile, tagFilters);
        }

    // true if url is good
    bool WARCPreprocessor::URLfilter(const std::string& url) {
        if (boost::algorithm::ends_with(url, "robots.txt"))
            return false;
        for (const std::string& ext : removeExtensions)
            if (boost::algorithm::ends_with(url, ext))
                return false;
        // if (url.find("google.translate") != std::string::npos)
        //     return false;
        return true;
    }


    void WARCPreprocessor::process(const std::string& filename) {
        BOOST_LOG_TRIVIAL(info) << "Processing " << filename;
        WARCReader reader(filename);

        std::string content;
        bool done = false;
        bool reliable;

        bool pdfpass = !pdf_warc_filename.empty();
        WARCWriter pdf_warc_writer;

        while (!done) {
            done = !reader.getRecord(content);
            if (done)
                continue;

            Record record(content);
            if (record.getPayload().empty())
                continue;

            if ((record.getRecordType() != "response" && record.getRecordType() != "resource") || record.getWARCcontentType().find("application/http") == std::string::npos)
                continue;

            if (boost::algorithm::ends_with(record.getURL(), ".pdf") or record.getHTTPcontentType() == "application/pdf") {
                // found a PDF file, write record to disk and continue
                if (pdfpass and not pdf_warc_writer.is_open()) pdf_warc_writer.open(pdf_warc_filename);
                if (pdfpass) pdf_warc_writer.writeRecord(content);
                continue;
            }

            if (textContentTypes.find(record.getHTTPcontentType()) == textContentTypes.end())
                continue;

            if (std::stoul(record.getHeaderProperty("Content-Length")) > 5242880)
                continue;

            if (!URLfilter(record.getURL()))
                continue;

            BOOST_LOG_TRIVIAL(trace) << "Processing HTML document " << record.getURL() << "\n";

            ++totalRecords;
            totalBytes += record.getPayload().size();

            int clean_retval = record.cleanPayload(tagFilters);
            if (clean_retval == util::FILTERED_DOCUMENT_ERROR) {
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
            }
            // TODO: decide what to do with other cases?

            if (record.getPlainText().empty()) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": empty";
                continue;
            }

            ++textRecords;
            textBytes += record.getPlainText().size();

            reliable = record.detectLanguage();
            if (!reliable) {
                BOOST_LOG_TRIVIAL(trace) << "Record " << record.getURL() << ": language not detected";
                continue;
            }

            ++langRecords;
            langBytes += record.getPlainText().size();

            writer.write(record);
        }
        pdf_warc_writer.close();
    }

    void WARCPreprocessor::printStatistics() const{
        BOOST_LOG_TRIVIAL(info) << "total records: " << totalRecords;
        BOOST_LOG_TRIVIAL(info) << "text records: " << textRecords;
        BOOST_LOG_TRIVIAL(info) << "lang records: " << langRecords;

        BOOST_LOG_TRIVIAL(info) << "total bytes: " << totalBytes;
        BOOST_LOG_TRIVIAL(info) << "text bytes: " << textBytes;
        BOOST_LOG_TRIVIAL(info) << "lang bytes: " << langBytes;
    }

    WARCWriter::WARCWriter() {
        warc = NULL;
    }

    void WARCWriter::open(const std::string& warc_filename) {
        filename = warc_filename;
        if (not boost::algorithm::ends_with(filename, ".warc.gz"))
            filename += ".warc.gz";
        std::string folder = filename.substr(0, filename.find_last_of('/'));
        util::createDirectories(folder);
        warc = std::fopen(filename.c_str(), "wb");
    }

    bool WARCWriter::is_open() {
        return warc != NULL;
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
