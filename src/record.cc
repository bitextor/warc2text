//
// Created by lpla on 5/10/20.
//

#include "record.hh"
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "xh_scanner.hh"

struct str_istream : public markup::instream {
    const char *p;
    const char *end;

    explicit str_istream(const char *src) : p(src), end(src + strlen(src)) {}

    wchar_t get_char() override { return p < end ? *p++ : 0; }
};

namespace {
    static std::unordered_set<std::string> startNL ( {"ul", "ol", "dl", "tr"} );
    static std::unordered_set<std::string> endNL ( {"p", "div", "li", "dd", "th", "td", "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9"} );
    static std::unordered_set<std::string> selfNL ( {"br"} );
    static std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} );
}

Record::Record(const std::string& content) {
    std::stringstream ss(content);
    std::string to;
    bool readingHeader = true;
    bool readingHTTPHeader = true;
    std::string delimiter = ": ";
    payload = "";
    while (std::getline(ss, to, '\n')) {
        std::string trimmed;
        if (readingHeader or readingHTTPHeader){
            trimmed = boost::trim_right_copy(to);
        }
        if (trimmed.empty() && readingHeader) {
            readingHeader = false;
        } else if (trimmed.empty() && readingHTTPHeader) {
            readingHTTPHeader = false;
        }

        if (readingHeader) {
            std::string key = trimmed.substr(0, trimmed.find(delimiter));
            std::string value = trimmed.substr(trimmed.find(delimiter) + 2, trimmed.length());
            header[key] = value;
        } else if (readingHTTPHeader) {
            if (trimmed.find(delimiter) != std::string::npos) {
                std::string key = trimmed.substr(0, trimmed.find(delimiter));
                std::string value = trimmed.substr(trimmed.find(delimiter) + 2, trimmed.length());
                HTTPheader[key] = value;
            }
        } else {
            payload = payload + to + "\n";
        }
    }
}

std::string Record::getHeaderProperty(const std::string& property) {
    if (header.find(property) != header.end()) {
        return header[property];
    } else {
        return "";
    }
}

std::string Record::getHTTPHeaderProperty(const std::string& property) {
    if (HTTPheader.find(property) != HTTPheader.end()) {
        return HTTPheader[property];
    } else {
        return "";
    }
}

std::string Record::getPayload() {
    return payload;
}

void Record::getPayloadPlainText(std::wstring &plaintext){
    str_istream si(payload.c_str());
    markup::scanner sc(si);
    const wchar_t *value;
    while (true) {
        int t = sc.get_token();
        std::unordered_set<std::string>::const_iterator got;
        std::unordered_set<std::string>::const_iterator gotSelf;
        switch (t) {
            case markup::scanner::TT_ERROR:
                //printf("ERROR\n");
                break;
            case markup::scanner::TT_EOF:
                //printf("EOF\n");
                goto FINISH;
            case markup::scanner::TT_TAG_START:
                got = startNL.find(sc.get_tag_name());
                if (got != startNL.end()) {
                    plaintext.append(L"\n");
                }
                //printf("TAG START:%s\n", sc.get_tag_name());
                break;
            case markup::scanner::TT_TAG_END:
                got = endNL.find(sc.get_tag_name());
                gotSelf = selfNL.find(sc.get_tag_name());
                if (got != endNL.end() or gotSelf != selfNL.end()) {
                    plaintext.append(L"\n");
                } else {
                    plaintext.append(L" ");
                }
                //printf("TAG END:%s\n", sc.get_tag_name());
                break;
            case markup::scanner::TT_ATTR:
                //printf("\tATTR:%s=%S\n", sc.get_attr_name(), sc.get_value());
                break;
            case markup::scanner::TT_WORD:
                got = noText.find(sc.get_tag_name());
                if (got == noText.end()) {
                    value = sc.get_value();
                    plaintext.append(value);
                    //printf("{%S}\n", value);
                }
                break;
            case markup::scanner::TT_SPACE:
                break;
            default:
                //printf("Unknown tag\n");
                break;
        }
    }
    FINISH:
    ;
    //printf("--------------------------\n");
}


std::unordered_map<std::string, std::string> Record::getHeader() {
    return header;
}

std::unordered_map<std::string, std::string> Record::getHTTPHeader() {
    return HTTPheader;
}
