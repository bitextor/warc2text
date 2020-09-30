#include <iostream>
#include <chrono>
#include "src/warc.hh"
#include <boost/algorithm/string.hpp>
#include <wordexp.h>


void PreProcessFile(const std::string& filename);

void PreProcessFile(const std::string &filename) {
    WARCReader reader(filename.c_str());
    std::string str;
    while (reader.getRecord(str)) {
        Record record = Record(str);
        std::string trimmed = boost::trim_copy(record.getPayload());
        trimmed.erase(std::remove(trimmed.begin(), trimmed.end(), '\r'), trimmed.end());
        if (!trimmed.empty()) {
            std::cout << record.getHeaderProperty("WARC-Target-URI") << std::endl;
            std::cout << record.getHTTPHeaderProperty("Date") << std::endl;
            std::cout << trimmed << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
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
