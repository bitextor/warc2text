//
// Created by lpla on 5/10/20.
//

#ifndef WARC2TEXT_RECORD_HH
#define WARC2TEXT_RECORD_HH

#include <string>
#include <unordered_map>
#include <regex>
#include "util.hh"
#include "lang.hh"

namespace warc2text {
    class Record {
    public:
        Record(const std::string& content, const std::string &filename, std::size_t size, std::size_t offset);
        const std::string& getHeaderProperty(const std::string& property) const;
        bool headerExists(const std::string& property) const;

        const std::string& getHTTPheaderProperty(const std::string& property) const;
        bool HTTPheaderExists(const std::string& property) const;

        const std::string& getPayload() const;
        const std::string& getPlainText() const;
        const std::string& getURL() const;
        const std::string& getRecordType() const;
        const std::string& getWARCcontentType() const;
        const std::string& getWARCdate() const;
        const std::string& getHTTPcontentType() const;
        const std::string& getCharset() const;
        bool isBroaderDocumentFormat() const;
        bool isTextFormat() const;

        inline const std::string& getFilename() const {
            return filename;
        }

        inline std::size_t getSize() const {
            return size;
        }

        inline std::size_t getOffset() const {
            return offset;
        }
        
        const std::unordered_map<std::string, std::string>& getTextByLangs() const;

        int cleanPayload();
        int cleanPayload(const util::umap_tag_filters_regex& tagFilters);
        int detectLanguage(LanguageDetector const &detector);

        static std::string readZipPayload(const std::string& content_type, const std::string& payload);
        static std::string isPayloadZip(const std::string& content_type, const std::string& uri);

        void encodeURL();

    private:
        const std::string &filename;
        std::size_t size; // compressed record length in WARC
        std::size_t offset; // byte offset of start of record in WARC

        std::unordered_map<std::string, std::string> header;
        std::unordered_map<std::string, std::string> HTTPheader;
        std::string payload;
        std::string plaintext;
        std::string language;

        std::unordered_map<std::string, std::string> text_by_langs;

        // these are present in the headers, but it's convenient to have them apart also
        std::string recordType;
        std::string WARCcontentType;
        std::string WARCdate;
        std::string cleanHTTPcontentType;
        std::string charset;
        std::string url;
        bool bdf_zip{};

        static const std::unordered_map<std::string, std::regex> zip_types;
        static const std::unordered_set<std::string> textContentTypes;

        void cleanContentType(const std::string& HTTPcontentType);
    };

} // warc2text

#endif //WARC2TEXT_RECORD_HH
