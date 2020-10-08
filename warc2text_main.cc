#include <iostream>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include "src/record.hh"
#include "src/warcreader.hh"

void PreProcessFile(const std::string& filename);

void PreProcessFile(const std::string &filename) {
    WARCReader reader(filename);
    std::string str;
    std::string url;
    while (reader.getRecord(str)) {
        Record record = Record(str);
        if (record.getHeaderProperty("WARC-Type") == "response") {
            std::string plaintext;
            record.getPayloadPlainText(plaintext);
            plaintext.erase(std::remove(plaintext.begin(), plaintext.end(), '\r'), plaintext.end());
            if (!plaintext.empty()) {
                std::cout << record.getHeaderProperty("WARC-Target-URI") << std::endl;
                if (record.HTTPheaderExists("Date"))
                    std::cout << record.getHTTPheaderProperty("Date") << std::endl;
                std::cout << plaintext << std::endl;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::string filename = "-";
    if (argc > 1) {
        filename = std::string(argv[1]);
    }
    PreProcessFile(filename);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    unsigned int elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

//    printf("total records: %d\n", proc.TotalRecords)
//    printf("text records: %d\n", proc.TextRecords)
//    printf("lang records: %d\n", proc.LangRecords)
//    printf("total bytes: %d\n", proc.TotalBytes)
//    printf("text bytes: %d\n", proc.TextBytes)
//    printf("lang bytes: %d\n", proc.LangBytes)
    std::cerr << "elapsed time: " << elapsed << "s\n";

    return 0;
}
