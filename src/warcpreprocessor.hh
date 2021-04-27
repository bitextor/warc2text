#ifndef WARC2TEXT_WARCPREPROCESSOR_HH
#define WARC2TEXT_WARCPREPROCESSOR_HH

#include "record.hh"
#include "warcreader.hh"
#include "bilangwriter.hh"
#include "util.hh"
#include <string>
#include <unordered_set>

namespace warc2text {
    class WARCWriter {
        private:
            FILE* warc;
            std::string filename;
        public:
            WARCWriter();
            void open(const std::string& warc_filename);
            void close();
            bool is_open();
            void writeRecord(const std::string& content);
    };

    class WARCPreprocessor {
        private:
            BilangWriter writer;
            unsigned int totalRecords;
            unsigned int textRecords;
            unsigned int langRecords;
            unsigned int totalBytes;
            unsigned int textBytes;
            unsigned int langBytes;
            util::umap_tag_filters_regex tagFilters;
            std::vector<util::umap_attr_regex> urlFilters;
            std::string pdf_warc_filename;
            bool invert;
            bool multilang;
            bool encodeURLs;

            static const std::unordered_set<std::string> removeExtensions;
            bool URLfilter(const std::string& url);

        public:
            explicit WARCPreprocessor(const std::string& outputFolder, const std::unordered_set<std::string>& output_files = {}, const std::string& pdf_warc_filename = "", const std::string& tagFiltersFile = "", bool invert = false, const std::string& urlFiltersFile = "", bool multilang = false, bool encodeURLs = false);
            void process(const std::string &filename);
            void printStatistics() const;
    };
}

#endif
