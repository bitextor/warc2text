#ifndef WARC2TEXT_WARCPREPROCESSOR_HH
#define WARC2TEXT_WARCPREPROCESSOR_HH

#include "record.hh"
#include "warcreader.hh"
#include "bilangwriter.hh"
#include "util.hh"
#include <string>
#include <unordered_set>

namespace warc2text {
    struct WARCPreprocOptions {
        std::string output_folder;
        std::unordered_set<std::string> files;
        std::string pdf_warc_filename;
        std::string tag_filters_file;
        bool tag_filters_invert{};
        bool multilang{};
        std::string pdfextract_jar;
        std::string pdfextract_config_file;
        std::string pdfextract_log_file;
    };

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
            std::string pdf_warc_filename;
            bool invert;
            bool multilang;
            bool pdfextract;

            static const std::unordered_set<std::string> removeExtensions;
            static bool URLfilter(const std::string& url);

        public:
            explicit WARCPreprocessor(const WARCPreprocOptions& options);
            ~WARCPreprocessor();
            void process(const std::string &filename);
            void printStatistics() const;
    };
}

#endif
