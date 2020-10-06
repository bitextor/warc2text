#include <iostream>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <wordexp.h>
#include "src/record.hh"
#include "src/warcreader.hh"

void PreProcessFile(const std::string& filename);

void PreProcessFile(const std::string &filename) {
    WARCReader reader(filename);
    std::string str;
    while (reader.getRecord(str)) {
        Record record = Record(str);
        if (record.getHeaderProperty("WARC-Type") == "response") {
            std::string plaintext;
            record.getPayloadPlainText(plaintext);
            plaintext.erase(std::remove(plaintext.begin(), plaintext.end(), '\r'), plaintext.end());
            if (!plaintext.empty()) {
                std::cout << record.getHeaderProperty("WARC-Target-URI") << std::endl;
                std::cout << record.getHTTPheaderProperty("Date") << std::endl;
                std::cout << plaintext << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::string filename = "-";
    if (argc > 1){
        wordexp_t exp_result;
        wordexp(argv[1], &exp_result, 0);
        filename = exp_result.we_wordv[0];
    }
    PreProcessFile(filename);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    unsigned char elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

//    printf("total records: %d\n", proc.TotalRecords)
//    printf("text records: %d\n", proc.TextRecords)
//    printf("lang records: %d\n", proc.LangRecords)
//    printf("total bytes: %d\n", proc.TotalBytes)
//    printf("text bytes: %d\n", proc.TextBytes)
//    printf("lang bytes: %d\n", proc.LangBytes)
    printf("elapsed time: %d\n", elapsed);

    return 0;
}
