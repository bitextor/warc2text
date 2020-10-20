#include "warcpreprocessor.hh"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace warc2text {
    const std::unordered_set<std::string> WARCPreprocessor::textContentTypes = {"text/plain", "text/html", "application/xml"};

    WARCPreprocessor::WARCPreprocessor(const std::string& outputFolder) : writer(outputFolder), totalRecords(0), textRecords(0), langRecords(0) {}


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
            if ((record.getRecordType() != "response" && record.getRecordType() != "resource") || record.getWARCcontentType().find("application/http") == std::string::npos)
                continue;

            if (textContentTypes.find(record.getHTTPcontentType()) == textContentTypes.end())
                continue;


            ++totalRecords;
            if (boost::algorithm::ends_with(record.getURL(), "robots.txt"))
                continue;

            record.cleanPayload();
            if (record.getPlainText().empty())
                continue;

            ++textRecords;

            reliable = record.detectLanguage();
            if (!reliable)
                continue;

            ++langRecords;

            writer.write(record);
        }
    }

    void WARCPreprocessor::printStatistics(){
        BOOST_LOG_TRIVIAL(info) << "total records: " << totalRecords;
        BOOST_LOG_TRIVIAL(info) << "text records: " << textRecords;
        BOOST_LOG_TRIVIAL(info) << "lang recods: " << langRecords;
    }

}
