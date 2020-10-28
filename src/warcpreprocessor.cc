#include "warcpreprocessor.hh"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace warc2text {
    const std::unordered_set<std::string> WARCPreprocessor::textContentTypes = {"text/plain", "text/html", "application/xml"};

    WARCPreprocessor::WARCPreprocessor(const std::string& outputFolder) :
        writer(outputFolder),
        totalRecords(0),
        textRecords(0),
        langRecords(0),
        totalBytes(0),
        textBytes(0),
        langBytes(0) {}


    void WARCPreprocessor::process(const std::string& filename) {
        BOOST_LOG_TRIVIAL(info) << "Processing " << filename;
        WARCReader reader(filename);

        std::string content;
        bool done = false;
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

            if (textContentTypes.find(record.getHTTPcontentType()) == textContentTypes.end())
                continue;


            ++totalRecords;
            totalBytes += record.getPayload().size();
            if (boost::algorithm::ends_with(record.getURL(), "robots.txt"))
                continue;

            record.cleanPayload();
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
