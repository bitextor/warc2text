#include "warcpreprocessor.hh"
#include <boost/algorithm/string/predicate.hpp>

namespace warc2text {
    void WARCPreprocessor::Process(const std::string& filename) {
        WARCReader reader(filename);

        std::string content;
        bool done = false;
        bool reliable;

        while (!done) {
            done = !reader.getRecord(content); 
            if (done)
                continue;
            Record record(content);
            if ((record.getRecordType() != "response" && record.getRecordType() != "resource") || record.getContentType().find("application/http") != std::string::npos)
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
}
