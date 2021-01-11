//
// Created by lpla on 5/10/20.
//

#ifndef WARC2TEXT_RECORD_HH
#define WARC2TEXT_RECORD_HH

#include <string>
#include <unordered_map>
#include "util.hh"
#include "regex"
#include "map"

namespace warc2text {
    class Record {
    public:
        Record() {};

        explicit Record(const std::string& content);
        const std::string& getHeaderProperty(const std::string& property) const;
        bool headerExists(const std::string& property) const;

        const std::string& getHTTPheaderProperty(const std::string& property) const;
        bool HTTPheaderExists(const std::string& property) const;

        const std::string& getPayload() const;
        const std::string& getPlainText() const;
        const std::string& getLanguage() const;
        const std::string& getURL() const;
        const std::string& getRecordType() const;
        const std::string& getWARCcontentType() const;
        const std::string& getHTTPcontentType() const;
        const bool& isBroaderDocumentFormat() const;
        static std::string readZipPayload(const std::string& content_type, const std::string& payload);
        const std::string& getCharset() const;

        int cleanPayload();
        int cleanPayload(const util::umap_tag_filters_regex& tagFilters);
        bool detectLanguage();
        static std::pair<std::string, bool> isPayloadZip(std::string content_type, const std::string& uri);

            private:
        std::unordered_map<std::string, std::string> header;
        std::unordered_map<std::string, std::string> HTTPheader;
        std::string payload;
        std::string plaintext;
        std::string language;

        // these are present in the headers, but it's convenient to have them apart also
        std::string recordType;
        std::string WARCcontentType;
        std::string cleanHTTPcontentType;
        std::string charset;
        std::string url;
        bool bdf_zip{};
        static std::map<std::string, std::regex> zip_types;

        static const std::unordered_set<std::string> textContentTypes;

        void cleanContentType(const std::string& HTTPcontentType);
    };

} // warc2text

#endif //WARC2TEXT_RECORD_HH
