#include <iostream>
#include <chrono>
#include "src/warc.hh"
#include <boost/algorithm/string.hpp>
#include <wordexp.h>
#define __unused __attribute__((unused))


void PreProcessFile(const std::string& filename);

void PreProcessFile(const std::string &filename) {
    WARCReader reader(filename);
    std::string str;
    while (reader.getRecord(str)) {
        Record record = Record(str);
        if (record.getHeader()["WARC-Type"] == "response") {
            std::wstring plaintext = L"";
            record.getPayloadPlainText(plaintext);
            plaintext.erase(std::remove(plaintext.begin(), plaintext.end(), '\r'), plaintext.end());
            if (!plaintext.empty()) {
                std::cout << record.getHeaderProperty("WARC-Target-URI") << std::endl;
                std::cout << record.getHTTPHeaderProperty("Date") << std::endl;
                std::wcout << plaintext << std::endl;
            }
        }
    }
}

int main(__unused int argc, char *argv[]) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    wordexp_t exp_result;
    wordexp(argv[1], &exp_result, 0);
    std::string filename = exp_result.we_wordv[0];
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
