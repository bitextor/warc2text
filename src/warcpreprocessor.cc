#include "warcpreprocessor.hh"
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace warc2text {
    WARCPreprocessor::WARCPreprocessor(const std::string& outputFolder) : writer(outputFolder) {}

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
            if (record.getRecordType() != "response" && record.getRecordType() != "resource")
                continue;

            ++totalRecords;
            // TODO: add url filter
            // TODO: add content type filters


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
