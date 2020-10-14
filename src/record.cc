//
// Created by lpla on 5/10/20.
//

#include "record.hh"
#include "util.hh"
#include "xh_scanner.hh"
#include <string.h>

extern "C" size_t decode_html_entities_utf8(char *dest, const char *src);

struct str_istream : public markup::instream {
    const char *p;
    const char *end;

    explicit str_istream(const char *src) : p(src), end(src + strlen(src)) {}

    char get_char() override { return p < end ? *p++ : 0; }
};

namespace {
    std::unordered_set<std::string> startNL ( {"ul", "ol", "dl", "tr"} );
    std::unordered_set<std::string> endNL ( {"p", "div", "li", "dd", "th", "td", "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9"} );
    std::unordered_set<std::string> selfNL ( {"br"} );
    std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} );
}

std::size_t read_header(const std::string& content, std::size_t last_pos, std::unordered_map<std::string,std::string>& header) {
    std::string line;
    std::size_t header_end = content.find("\r\n\r\n", last_pos);
    std::size_t pos = 0;
    if (header_end == std::string::npos) return std::string::npos;
    pos = content.find(':', last_pos);
    while (pos < header_end){
        line = content.substr(last_pos, pos - last_pos);
        pos = content.find_first_not_of(' ', pos + 1);
        last_pos = pos;
        pos = content.find("\r\n", pos);
        header[line] = content.substr(last_pos, pos - last_pos);
        last_pos = pos + 2;
        pos = content.find(':', last_pos);
    }
    return header_end + 4;
}

Record::Record(const std::string& content) {
    std::string line;
    std::size_t last_pos = 0, payload_start = 0;
    std::size_t pos = content.find("WARC/1.0\r\n");
    // TODO: throw proper exceptions
    if (pos != 0) throw "Error while parsing WARC header: version line not found";
    last_pos = pos + 10;
    // parse WARC header
    last_pos = read_header(content, last_pos, header);
    if (last_pos == std::string::npos) throw "Error while parsing WARC header";
    payload_start = last_pos;
    // TODO: check for mandatory header fields
    if (header["WARC-Type"] == "response") {
        // parse HTTP header
        pos = content.find("HTTP/1.", last_pos);
        if (pos == last_pos) { // found HTTP header
            pos = content.find("\r\n", last_pos);
            payload_start = read_header(content, pos + 2, HTTPheader);
            if (payload_start == std::string::npos)
                payload_start = last_pos; // could not parse the header, so treat it as part of the payload
                //throw "Error while parsing HTTP header";
        }
    }
    // read payload
    payload = std::string(content, payload_start, content.size() - last_pos - 4); // the -4 is for \r\n\r\n at the end
}


void Record::cleanPayload(){
    str_istream si(payload.c_str());
    markup::scanner sc(si);
    const char *value;
    int t = markup::scanner::TT_SPACE;
    std::unordered_set<std::string>::const_iterator got, gotSelf;
    while (t != markup::scanner::TT_EOF) {
        t = sc.get_token();
        switch (t) {
            case markup::scanner::TT_ERROR:
            case markup::scanner::TT_EOF:
                break;
            case markup::scanner::TT_TAG_START:
                got = startNL.find(sc.get_tag_name());
                if (got != startNL.end()) {
                    plaintext.append("\n");
                }
                break;
            case markup::scanner::TT_TAG_END:
                got = endNL.find(sc.get_tag_name());
                gotSelf = selfNL.find(sc.get_tag_name());
                if (got != endNL.end() or gotSelf != selfNL.end()) {
                    plaintext.append("\n");
                } else {
                    plaintext.append(" ");
                }
                break;
            case markup::scanner::TT_ATTR:
                break;
            case markup::scanner::TT_WORD:
                got = noText.find(sc.get_tag_name());
                if (got == noText.end()) {
                    value = sc.get_value();
                    if (strcmp(value,"&nbsp;") != 0) {
                        plaintext.append(value);
                    }
                }
                break;
            case markup::scanner::TT_SPACE:
                plaintext.append(" ");
                break;
            default:
                break;
        }
    }
    char * decodedplaintext = new char [plaintext.size() + 1];
    decode_html_entities_utf8(decodedplaintext, plaintext.c_str());
    plaintext = decodedplaintext;
    delete[] (decodedplaintext);

    util::trimLines(plaintext);
}


// std::unordered_map<std::string, std::string> Record::getHeader() {
//     return header;
// }
//
// std::unordered_map<std::string, std::string> Record::getHTTPHeader() {
//     return HTTPheader;
// }



const std::string& Record::getHeaderProperty(const std::string& property) const {
    return header.at(property);
}
bool Record::headerExists(const std::string& property) const {
    return header.find(property) != header.end();
}

const std::string& Record::getHTTPheaderProperty(const std::string& property) const {
    return HTTPheader.at(property);
}
bool Record::HTTPheaderExists(const std::string& property) const{
    return HTTPheader.find(property) != HTTPheader.end();
}

const std::string& Record::getPayload() const {
    return payload;
}

const std::string& Record::getPlainText() const {
    return plaintext;
}

