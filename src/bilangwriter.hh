#ifndef WARC2TEXT_WRITER_HH
#define WARC2TEXT_WRITER_HH

#include <fstream>
#include <unordered_map>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include "record.hh"

namespace warc2text {
    bool createDirectories(const std::string& path);

    class GzipWriter {
        private:
            std::ofstream file;
            boost::iostreams::filtering_streambuf<boost::iostreams::output> outbuf;
            std::ostream outstream;

        public:
            GzipWriter();
            ~GzipWriter();
            void open(const std::string& filename);
            void write(const char* text, unsigned int size);
            bool is_open();
    };

    class BilangWriter {
        private:
            std::string folder;
            std::unordered_map<std::string, GzipWriter> url_files;
            std::unordered_map<std::string, GzipWriter> mime_files;
            std::unordered_map<std::string, GzipWriter> text_files;
            // TODO make html output optional
            std::unordered_map<std::string, GzipWriter> html_files;
            std::unordered_map<std::string, GzipWriter> deferred_files;

        public:
            explicit BilangWriter(const std::string& folder) :
                folder(folder),
                url_files(),
                mime_files(),
                text_files(),
                html_files(),
                deferred_files()
            {};

            void write(const Record& record);
    };


}

#endif
