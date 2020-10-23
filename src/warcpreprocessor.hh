#ifndef WARC2TEXT_WARCPREPROCESSOR_HH
#define WARC2TEXT_WARCPREPROCESSOR_HH

#include "record.hh"
#include "warcreader.hh"
#include "bilangwriter.hh"
#include <string>
#include <unordered_set>

namespace warc2text {
    class WARCPreprocessor {
        private:
            BilangWriter writer;
            unsigned int totalRecords;
            unsigned int textRecords;
            unsigned int langRecords;
            unsigned int totalBytes;
            unsigned int textBytes;
            unsigned int langBytes;
            static const std::unordered_set<std::string> textContentTypes;

        public:
            WARCPreprocessor(const std::string& outputFolder);
            void process(const std::string &filename);
            void printStatistics();
    };
}

#endif
