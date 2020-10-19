#ifndef WARC2TEXT_WARCPREPROCESSOR_HH
#define WARC2TEXT_WARCPREPROCESSOR_HH

#include "record.hh"
#include "warcreader.hh"
#include "bilangwriter.hh"
#include <string>

namespace warc2text {
    class WARCPreprocessor {
        private:
            BilangWriter writer;
            unsigned int totalRecords;
            unsigned int textRecords;
            unsigned int langRecords;
            std::string outputFolder;

        public:
            WARCPreprocessor(const std::string& outputFolder) : writer(outputFolder) {}
            void Process(const std::string &filename);
    };
}

#endif
