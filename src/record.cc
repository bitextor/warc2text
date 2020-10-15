//
// Created by lpla on 5/10/20.
//

#include "record.hh"
#include "util.hh"
#include "lang.hh"
#include "html.hh"

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
    warc2text::processHTML(payload, plaintext);
    warc2text::unescapeEntities(plaintext, plaintext);
    util::trimLines(plaintext);
}

bool Record::detectLanguage(){
    return warc2text::detectLanguage(plaintext, language);
}

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

const std::string& Record::getLanguage() const {
    return language;
}
