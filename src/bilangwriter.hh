#ifndef WARC2TEXT_WRITER_HH
#define WARC2TEXT_WRITER_HH

#include <unordered_map>
#include <boost/filesystem.hpp>
#include "record.hh"
#include "zlib.h"

namespace warc2text {
    bool createDirectories(const std::string& path);

    class GzipWriter {
        private:
            FILE* dest;
            z_stream s;
            unsigned char* buf;
            std::size_t compressed;
            void compress(const char* in, std::size_t size, int flush);

        public:
            GzipWriter();
            ~GzipWriter();
            void open(const std::string& filename);
            void write(const char* text, std::size_t size);
            void writeLine(const char* text, std::size_t size);
            bool is_open();
            static const std::size_t BUFFER_SIZE = 4096;
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
