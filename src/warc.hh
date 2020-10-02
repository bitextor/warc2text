#ifndef WARC_HH
#define WARC_HH

#include <iostream>
#include "zlib.h"
#include <map>
#include <vector>

class Record {
public:
    explicit Record(const std::string& content);

    std::string getHeaderProperty(const std::string& property);

    std::string getHTTPHeaderProperty(const std::string& property);

    std::string getPayload();

    std::map<std::string, std::string> getHeader();

    std::map<std::string, std::string> getHTTPHeader();

private:
    std::map<std::string, std::string> header;
    std::map<std::string, std::string> HTTPheader;
    std::string payload;

};

class WARCReader {
    public:
        explicit WARCReader(const std::string& filename);
        bool getRecord(std::string& out);
        ~WARCReader();
    private:
        std::FILE* file;
        z_stream s{};
        static const std::size_t BUFFER_SIZE = 1024;
        uint8_t* buf;
        uint8_t* scratch;

        void openFile(const std::string& filename);
        void closeFile() {std::fclose(file);}
        std::size_t readChunk(); 
};

#endif
