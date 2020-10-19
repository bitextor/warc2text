#include "warcpreprocessor.hh"

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
}
