#include <iostream>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include "src/record.hh"
#include "src/warcreader.hh"
#include "cld2/public/compact_lang_det.h"

using namespace warc2text;

void PreProcessFile(const std::string &filename) {
    WARCReader reader(filename);
    std::string content;
    auto start = std::chrono::steady_clock::now();
    auto end = start;
    auto warc_reading = start - end;
    auto record_parsing = warc_reading;
    auto html_cleaning = warc_reading;
    auto lang_detection = warc_reading;

    bool done = !reader.getRecord(content);
    std::string plaintext;
    while (!done) {
        start = std::chrono::steady_clock::now();
        Record record = Record(content);
        end = std::chrono::steady_clock::now();
        record_parsing += (end - start);
        if (record.getHeaderProperty("WARC-Type") == "response") {
            start = std::chrono::steady_clock::now();
            record.cleanPayload();
            plaintext = record.getPlainText();
            end = std::chrono::steady_clock::now();
            html_cleaning += (end - start);
            if (!plaintext.empty()) {
                std::cout << record.getHeaderProperty("WARC-Target-URI") << std::endl;
                if (record.HTTPheaderExists("Content-Type")) {
                    std::string cleanContentType = boost::algorithm::to_lower_copy(record.getHTTPheaderProperty("Content-Type"));
                    std::cout << cleanContentType.substr(0, cleanContentType.find(';')) << std::endl;
                }

                start = std::chrono::steady_clock::now();
                record.detectLanguage();
                end = std::chrono::steady_clock::now();
                lang_detection += (end - start);
                std::cout << record.getLanguage() << std::endl;
                std::cout << plaintext << std::endl;
            }
        }
        start = std::chrono::steady_clock::now();
        done = !reader.getRecord(content);
        end = std::chrono::steady_clock::now();
        warc_reading += (end - start);
    }
    std::cerr << "warc reading: " << std::chrono::duration_cast<std::chrono::seconds>(warc_reading).count() << "s\n";
    std::cerr << "record parsing: " << std::chrono::duration_cast<std::chrono::seconds>(record_parsing).count() << "s\n";
    std::cerr << "html cleaning: " << std::chrono::duration_cast<std::chrono::seconds>(html_cleaning).count() << "s\n";
    std::cerr << "lang detection: " << std::chrono::duration_cast<std::chrono::seconds>(lang_detection).count() << "s\n";
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
