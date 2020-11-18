#include "warcpreprocessor.hh"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace warc2text {
    const std::unordered_set<std::string> WARCPreprocessor::textContentTypes = {"text/plain", "text/html", "application/xml"};

    WARCPreprocessor::WARCPreprocessor(const std::string& outputFolder, const std::unordered_set<std::string>& output_files, const std::string& tagFiltersFile) :
        writer(outputFolder, output_files),
        totalRecords(0),
        textRecords(0),
        langRecords(0),
        totalBytes(0),
        textBytes(0),
        langBytes(0),
        tagFilters(),
        output_files(output_files) {
            if (!tagFiltersFile.empty())
                util::readTagFilters(tagFiltersFile, tagFilters);
        }


    void WARCPreprocessor::process(const std::string& filename) {
        BOOST_LOG_TRIVIAL(info) << "Processing " << filename;
        WARCReader reader(filename);

        std::string content;
        bool done = false;
        bool extractStandoff = output_files.count("deferred");
        bool reliable;

        while (!done) {
            done = !reader.getRecord(content);
            if (done)
                continue;

            Record record(content);
            if (record.getPayload().empty())
                continue;

            if ((record.getRecordType() != "response" && record.getRecordType() != "resource") || record.getWARCcontentType().find("application/http") == std::string::npos)
                continue;

            BOOST_LOG_TRIVIAL(trace) << "Processing HTML document " << record.getURL() << "\n";

            if (textContentTypes.find(record.getHTTPcontentType()) == textContentTypes.end())
                continue;


            ++totalRecords;
            totalBytes += record.getPayload().size();
            if (boost::algorithm::ends_with(record.getURL(), "robots.txt"))
                continue;

            int clean_retval = record.cleanPayload(extractStandoff, tagFilters);
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

            if (record.getPlainText().empty())
                continue;

            ++textRecords;
            textBytes += record.getPlainText().size();

            reliable = record.detectLanguage();
            if (!reliable)
                continue;

            ++langRecords;
            langBytes += record.getPlainText().size();

            writer.write(record);
        }
    }

    void WARCPreprocessor::printStatistics() const{
        BOOST_LOG_TRIVIAL(info) << "total records: " << totalRecords;
        BOOST_LOG_TRIVIAL(info) << "text records: " << textRecords;
        BOOST_LOG_TRIVIAL(info) << "lang records: " << langRecords;

        BOOST_LOG_TRIVIAL(info) << "total bytes: " << totalBytes;
        BOOST_LOG_TRIVIAL(info) << "text bytes: " << textBytes;
        BOOST_LOG_TRIVIAL(info) << "lang bytes: " << langBytes;
    }

}
