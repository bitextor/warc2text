#ifndef WARC2TEXT_WARCREADER_HH
#define WARC2TEXT_WARCREADER_HH

#include "zlib.h"
#include <string>

namespace warc2text {
    class WARCReader {
        public:
            WARCReader();
            explicit WARCReader(const std::string& filename);
            bool getRecord(std::string& out, std::size_t max_size = 1024*1024*20); //20MB
            std::size_t tell() const;
            ~WARCReader();
        private:
            std::FILE* file;
            std::string warc_filename;
            z_stream s{};
            static const std::size_t BUFFER_SIZE = 4096;
            uint8_t* buf;
            uint8_t* scratch;

            void openFile(const std::string& filename);
            void closeFile();
            std::size_t readChunk();
    };
}

#endif
