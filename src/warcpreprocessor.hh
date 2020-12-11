#ifndef WARC2TEXT_WARCPREPROCESSOR_HH
#define WARC2TEXT_WARCPREPROCESSOR_HH

#include "record.hh"
#include "warcreader.hh"
#include "bilangwriter.hh"
#include "util.hh"
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
            util::umap_tag_filters tagFilters;
            std::unordered_set<std::string> output_files;
            static const std::unordered_set<std::string> textContentTypes;
            static const std::unordered_set<std::string> removeExtensions;
            static bool URLfilter(const std::string& url);

        public:
            explicit WARCPreprocessor(const std::string& outputFolder, const std::unordered_set<std::string>& output_files = {}, const std::string& tagFiltersFile = "");
            void process(const std::string &filename);
            void printStatistics() const;
    };
}

#endif
