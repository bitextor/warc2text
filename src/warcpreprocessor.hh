#ifndef WARC2TEXT_WARCPREPROCESSOR_HH
#define WARC2TEXT_WARCPREPROCESSOR_HH

#include "record.hh"
#include "src/lang.hh"
#include "warcreader.hh"
#include "bilangwriter.hh"
#include "util.hh"
#include <memory>
#include <string>
#include <unordered_set>
#include <boost/regex.hpp>

namespace warc2text {
    class WARCWriter {
        private:
            FILE* warc;
            std::string filename;
        public:
            WARCWriter();
            ~WARCWriter();
            void open(const std::string& warc_filename);
            void close();
            bool is_open();
            void writeRecord(const std::string& content);
    };

    struct WARCPreprocessorOptions {
        std::string pdf_warc_filename;
        std::string robots_warc_filename;
        
        bool paragraph_identification{};

        std::string output;
        std::unordered_set<std::string> output_files;
        
        std::string tag_filters_filename;
        bool tag_filters_invert{};
        
        std::string url_filters_filename;
        
        bool multilang{};
        bool encodeURLs{};
    };

    class WARCPreprocessor {
        private:
            RecordWriter &writer;
            LanguageDetector const &detector;
            WARCPreprocessorOptions const &options;
            WARCWriter pdf_warc_writer;
            WARCWriter robots_warc_writer;
            unsigned int totalRecords;
            unsigned int textRecords;
            unsigned int langRecords;
            unsigned int totalBytes;
            unsigned int textBytes;
            unsigned int langBytes;
            util::umap_tag_filters_regex tagFilters;
            boost::regex urlFilter;
            
            static const std::unordered_set<std::string> removeExtensions;
            bool URLfilter(const std::string& url) const;

        public:
            explicit WARCPreprocessor(RecordWriter &writer, LanguageDetector const &detector, WARCPreprocessorOptions const &options);
            void process(const std::string &filename);
            void printStatistics() const;
    };
}

#endif
