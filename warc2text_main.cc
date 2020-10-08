#include <iostream>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <wordexp.h>
#include "src/record.hh"
#include "src/warcreader.hh"
#include "cld2/public/compact_lang_det.h"
#include "cld2/public/encodings.h"

void PreProcessFile(const std::string& filename);

void PreProcessFile(const std::string &filename) {
    WARCReader reader(filename);
    std::string content;
    while (reader.getRecord(content)) {
        Record record = Record(content);
        if (record.getHeaderProperty("WARC-Type") == "response") {
            std::string plaintext;
            record.getPayloadPlainText(plaintext);
            plaintext.erase(std::remove(plaintext.begin(), plaintext.end(), '\r'), plaintext.end());
            if (!plaintext.empty()) {
                std::cout << record.getHeaderProperty("WARC-Target-URI") << std::endl;
                std::cout << record.getHTTPheaderProperty("Content-Type") << std::endl;

                CLD2::CLDHints hints = {nullptr, nullptr, CLD2::UNKNOWN_ENCODING, CLD2::UNKNOWN_LANGUAGE};
                // cld2 output
                CLD2::ResultChunkVector chunks;
                bool reliable = false;
                CLD2::Language l = CLD2::DetectLanguage(plaintext.data(), plaintext.size(), true, &reliable);
                std::cout << CLD2::LanguageCode(l) << std::endl;

                // Testing code for language detection chunks in a document
                //for (auto chunk : chunks)
                //    std::cout << CLD2::LanguageCode( (CLD2::Language) chunk.lang1) << " " << plaintext.substr(chunk.offset, chunk.bytes) << std::endl; // substr makes a copy, don't use it in production

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
