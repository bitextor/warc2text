//
// Created by lpla on 5/10/20.
//

#include "record.hh"
#include "lang.hh"
#include "html.hh"
#include "util.hh"
#include "entities.hh"
#include "zipreader.hh"
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace warc2text {
    const std::unordered_set<std::string> Record::textContentTypes = {"text/plain", "text/html", "application/xml", "text/vnd.wap.wml", "application/atom+xml", "application/opensearchdescription+xml", "application/rss+xml", "application/xhtml+xml"};

    std::size_t read_header(const std::string& content, std::size_t last_pos, std::unordered_map<std::string,std::string>& header) {
        std::string line;
        std::size_t header_end = content.find("\r\n\r\n", last_pos);
        std::size_t pos;
        if (header_end == std::string::npos) return std::string::npos;
        pos = content.find(':', last_pos);
        while (pos < header_end){
            line = content.substr(last_pos, pos - last_pos);
            pos = content.find_first_not_of(' ', pos + 1);
            last_pos = pos;
            pos = content.find("\r\n", pos);
            // normalize header keys case
            util::toLower(line);
            header[line] = content.substr(last_pos, pos - last_pos);
            util::toLower(header[line]);
            last_pos = pos + 2;
            pos = content.find(':', last_pos);
        }
        return header_end + 4;
    }

    Record::Record(const std::string& content) {
        std::string line;
        std::size_t last_pos = 0, payload_start = 0;
        std::size_t pos = content.find("WARC/1.0\r\n");
        if (pos != 0) {
            BOOST_LOG_TRIVIAL(error) << "WARC version line not found";
            return;
        }
        last_pos = pos + 10;
        // parse WARC header
        last_pos = read_header(content, last_pos, header);

        if (last_pos == std::string::npos) {
            BOOST_LOG_TRIVIAL(error) << "Could not parse WARC header";
            return;
        }

        // get the most important stuff:
        // TODO: check for mandatory header fields
        if (header.count("warc-type") == 1)
            recordType = header["warc-type"];

        if (header.count("warc-target-uri") == 1)
            url = header["warc-target-uri"];

        if (!url.empty() && url[0] == '<' && url[url.size()-1] == '>')
            url = url.substr(1, url.size()-2);

        if (header.count("content-type") == 1)
            WARCcontentType = header["content-type"];

        payload_start = last_pos;
        if (header["warc-type"] == "response") {
            // parse HTTP header
            pos = content.find("HTTP/1.", last_pos);
            if (pos == last_pos) { // found HTTP header
                pos = content.find("\r\n", last_pos);
                payload_start = read_header(content, pos + 2, HTTPheader);
                if (payload_start == std::string::npos) {
                    // BOOST_LOG_TRIVIAL(warning) << "Response record without HTTP header";
                    payload_start = last_pos; // could not parse the header, so treat it as part of the payload
                }
            }
            // else {
            //     BOOST_LOG_TRIVIAL(warning) << "Response record without HTTP header";
            // }

            if (HTTPheader.count("content-type") == 1)
                cleanContentType(HTTPheader["content-type"]);
        }

        payload = std::string(content, payload_start, std::string::npos);

        util::trim(payload); //remove \r\n\r\n at the end

    }

    const std::unordered_map<std::string, std::regex> Record::zip_types = {
            {"application/vnd.oasis.opendocument.text",                                   std::regex("^content\\.xml$")},
            {"application/vnd.oasis.opendocument.spreadsheet",                            std::regex("^content\\.xml$")},
            {"application/vnd.oasis.opendocument.presentation",                           std::regex("^content\\.xml$")},
            {"application/vnd.openxmlformats-officedocument.wordprocessingml.document",   std::regex("^word/document\\.xml$")},
            {"application/vnd.openxmlformats-officedocument.presentationml.presentation", std::regex("^ppt/slides/slide.*$")},
            {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",         std::regex("^xl/sharedStrings\\.xml$")},
            {"application/epub+zip",                                                      std::regex("^.*ml$")}
    };

    std::pair<std::string, bool> Record::isPayloadZip(const std::string& content_type, const std::string& uri){

        if (boost::algorithm::ends_with(uri, ".odt")) {
            return std::make_pair("application/vnd.oasis.opendocument.text", true);
        }
        if (boost::algorithm::ends_with(uri, ".ods")) {
            return std::make_pair("application/vnd.oasis.opendocument.spreadsheet", true);
        }
        if (boost::algorithm::ends_with(uri, ".odp")) {
            return std::make_pair("application/vnd.oasis.opendocument.presentation", true);
        }
        if (boost::algorithm::ends_with(uri, ".docx")) {
            return std::make_pair("application/vnd.openxmlformats-officedocument.wordprocessingml.document", true);
        }
        if (boost::algorithm::ends_with(uri, ".pptx")) {
            return std::make_pair("application/vnd.openxmlformats-officedocument.presentationml.presentation", true);
        }
        if (boost::algorithm::ends_with(uri, ".xslx")) {
            return std::make_pair("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", true);
        }
        if (boost::algorithm::ends_with(uri, ".epub")) {
            return std::make_pair("application/epub+zip", true);
        }

        if (zip_types.count(content_type)){
            return std::make_pair(content_type, true);
        }

        return std::make_pair(content_type, false);

    }

    std::string Record::readZipPayload(const std::string& content_type, const std::string& payload){
        std::string unzipped_payload;
        
        util::ZipReader zip(payload);

        for (auto file : zip)
            if (std::regex_match(file.name(), zip_types.at(content_type)))
                unzipped_payload += file.read();

        return unzipped_payload;
    }

    void Record::cleanContentType(const std::string& HTTPcontentType) {
        // we assume the format is either "A/B; charset=C" or just "A/B"
        std::size_t delim = HTTPcontentType.find(';');
        if (delim == std::string::npos)
            cleanHTTPcontentType = HTTPcontentType;
        else {
            cleanHTTPcontentType = HTTPcontentType.substr(0, delim);
            delim = HTTPcontentType.find("charset=");
            if (delim != std::string::npos) {
                // cut until next ';' or until the end otherwise
                charset = HTTPcontentType.substr(delim+8, HTTPcontentType.find(';', delim+8) - delim - 8);
                util::trim(charset);
            }
        }
        // trim just in case
        util::trim(cleanHTTPcontentType);
    }

    int Record::cleanPayload(){
        util::umap_tag_filters_regex tagFilters;
        return cleanPayload(tagFilters);
    }

    int Record::cleanPayload(const util::umap_tag_filters_regex& tagFilters){

        std::string content_type;
        std::tie(content_type, bdf_zip) = isPayloadZip(cleanHTTPcontentType, url);
        if (textContentTypes.find(cleanHTTPcontentType) == textContentTypes.end() && !bdf_zip)
            return util::NOT_VALID_RECORD;
        if (bdf_zip)
            payload = readZipPayload(content_type, payload);

        // detect charset
        std::string detected_charset;
        std::string extracted;
        bool detection_result = util::detectCharset(payload, detected_charset, charset);

        if (detection_result) charset = detected_charset;
        // throw out documents if we don't know the charset
        else return util::UNKNOWN_ENCODING_ERROR;

        // remove HTML tags:
        int retval = processHTML(payload, extracted, tagFilters);

        // convert to utf8 if needed:
        bool needToConvert = !(charset == "utf8" or charset == "utf-8" or charset == "ascii");
        if (needToConvert) {
            extracted = util::toUTF8(extracted, charset);
        }

        // decode HTML entities:
        entities::decodeEntities(extracted, plaintext);

        return retval;
    }

    bool Record::detectLanguage(){
        return warc2text::detectLanguage(plaintext, language);
    }

    const std::string& Record::getHeaderProperty(const std::string& property) const {
        std::string lc_key = property;
        util::toLower(lc_key);
        return header.at(lc_key);
    }
    bool Record::headerExists(const std::string& property) const {
        std::string lc_key = property;
        util::toLower(lc_key);
        return header.find(lc_key) != header.end();
    }

    const std::string& Record::getHTTPheaderProperty(const std::string& property) const {
        std::string lc_key = property;
        util::toLower(lc_key);
        return HTTPheader.at(lc_key);
    }

    bool Record::HTTPheaderExists(const std::string& property) const{
        std::string lc_key = property;
        util::toLower(lc_key);
        return HTTPheader.find(lc_key) != HTTPheader.end();
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

    const std::string& Record::getURL() const {
        return url;
    }

    const std::string& Record::getRecordType() const {
        return recordType;
    }

    const std::string& Record::getWARCcontentType() const {
        return WARCcontentType;
    }

    const std::string& Record::getHTTPcontentType() const {
        return cleanHTTPcontentType;
    }

    const std::string& Record::getCharset() const {
        return charset;
    }

    bool Record::isBroaderDocumentFormat() const {
        return bdf_zip;
    }

} // warc2text
