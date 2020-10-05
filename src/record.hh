//
// Created by lpla on 5/10/20.
//

#ifndef WARC2TEXT_RECORD_HH
#define WARC2TEXT_RECORD_HH

#include <string>
#include <unordered_map>
#include <unordered_set>

class Record {
public:
    explicit Record(const std::string& content);

    std::string getHeaderProperty(const std::string& property);

    std::string getHTTPHeaderProperty(const std::string& property);

    std::string getPayload();

    void getPayloadPlainText(std::wstring &plaintext);

    std::unordered_map<std::string, std::string> getHeader();

    std::unordered_map<std::string, std::string> getHTTPHeader();

private:
    std::unordered_map<std::string, std::string> header;
    std::unordered_map<std::string, std::string> HTTPheader;
    std::string payload;
};

#endif //WARC2TEXT_RECORD_HH
