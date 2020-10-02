#include "warc.hh"
#include <cassert>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "xh_scanner.h"

struct str_istream : public markup::instream {
    const char *p;
    const char *end;

    explicit str_istream(const char *src) : p(src), end(src + strlen(src)) {}

    wchar_t get_char() override { return p < end ? *p++ : 0; }
};

WARCReader::WARCReader(const std::string& filename){
    file = nullptr;

    openFile(filename);

    buf = new uint8_t[BUFFER_SIZE];
    scratch = new uint8_t[BUFFER_SIZE];
    
    s.zalloc = nullptr;
    s.zfree = nullptr;
    s.opaque = nullptr;
    s.avail_in = 0;
    s.next_in = buf;

    assert(inflateInit2(&s, 32) == Z_OK);
}

WARCReader::~WARCReader(){
    delete[] buf;
    delete[] scratch;
    inflateEnd(&s);
    closeFile();
}

bool WARCReader::getRecord(std::string& out){
    int inflate_ret = 0;
    out.clear();
    std::size_t len;
    while (inflate_ret != Z_STREAM_END) {
        if (s.avail_in == 0) {
            len = readChunk();
            if (len <= 0) {
                // nothing more to read
                out.clear();
                return false;
            }
            s.avail_in = len;
            s.next_in = buf;
        }
        // inflate until either stream end is reached, or there is no more data
        while (inflate_ret != Z_STREAM_END && s.avail_in != 0) {
            s.next_out = scratch;
            s.avail_out = BUFFER_SIZE;
            inflate_ret = inflate(&s, Z_NO_FLUSH);
            assert(inflate_ret == Z_OK || inflate_ret == Z_STREAM_END);
            out.append(scratch, scratch + (BUFFER_SIZE - s.avail_out));
        }
        if (inflate_ret == Z_STREAM_END) {
            assert(inflateReset(&s) == Z_OK);
            // next in and avail_in are updated while inflating, so no need to update them manually
        }
    }
    return true;
}

void WARCReader::openFile(const std::string& filename){
    if (filename.empty() || filename == "-")
        file = std::freopen(nullptr, "rb", stdin); // make sure stdin is open in binary mode
    else file = std::fopen(filename.c_str(), "r");
    if (!file) {
        std::perror("File opening failed");
        exit(1);
    }
}

std::size_t WARCReader::readChunk(){
    std::size_t len = std::fread(buf, sizeof(uint8_t), BUFFER_SIZE, file);
    if (std::ferror(file) && !std::feof(file)) {
        std::perror("Error during reading");
        exit(1);
    }
    return len;
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
        switch (t) {
            case markup::scanner::TT_ERROR:
                //printf("ERROR\n");
                break;
            case markup::scanner::TT_EOF:
                //printf("EOF\n");
                goto FINISH;
            case markup::scanner::TT_TAG_START:
                //printf("TAG START:%s\n", sc.get_tag_name());
                break;
            case markup::scanner::TT_TAG_END:
                //printf("TAG END:%s\n", sc.get_tag_name());
                break;
            case markup::scanner::TT_ATTR:
                //printf("\tATTR:%s=%S\n", sc.get_attr_name(), sc.get_value());
                break;
            case markup::scanner::TT_WORD:
                value = sc.get_value();
                plaintext.append(value);
                plaintext.append(L" ");
                //printf("{%S}\n", value);
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


std::map<std::string, std::string> Record::getHeader() {
    return header;
}

std::map<std::string, std::string> Record::getHTTPHeader() {
    return HTTPheader;
}