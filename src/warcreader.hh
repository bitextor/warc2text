#ifndef WARC_HH
#define WARC_HH

#include "zlib.h"
#include <string>

class WARCReader {
    public:
        explicit WARCReader(const std::string& filename);
        bool getRecord(std::string& out);
        ~WARCReader();
    private:
        std::FILE* file;
        z_stream s{};
        static const std::size_t BUFFER_SIZE = 4096;
        uint8_t* buf;
        uint8_t* scratch;

        void openFile(const std::string& filename);
        void closeFile() {std::fclose(file);}
        std::size_t readChunk(); 
};

#endif
