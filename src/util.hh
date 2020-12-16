#ifndef WARC2TEXT_UTIL_HH
#define WARC2TEXT_UTIL_HH

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace util {
    void toLower(std::string& s);
    std::string toLowerCopy(const std::string& s);

    // trim consecutive spaces from left and right
    void trim(std::string& s);

    // trim consecutive spaces but respect newlines:
    void trimLines(std::string& text);
    void trimLinesCopy(const std::string& original, std::string& result);

    // detect charset using uchardet
    bool detectCharset(const std::string& text, std::string& charset, const std::string& original_charset = "");
    // convert to utf8
    std::string toUTF8 (const std::string& text, const std::string& charset);
    std::string toUTF8 (const char* text, const std::string& charset);

    void encodeBase64(const std::string& original, std::string& base64);

    void decodeBase64(const std::string& base64, std::string& output);

    enum ErrorCode : int {
        SUCCESS = 0,
        HTML_PARSING_ERROR = 1,
        FILTERED_DOCUMENT_ERROR = 2,
        UNKNOWN_ENCODING_ERROR = 3,
        UTF8_CONVERSION_ERROR = 4
    };

    inline bool uset_contains(const std::unordered_set<std::string>& uset, const std::string& value) {
        return uset.find(value) != uset.end();
    }

    typedef std::unordered_map<std::string, std::vector<std::string>> umap_attr_filters;
    typedef std::unordered_map<std::string, umap_attr_filters> umap_tag_filters;

    void readTagFilters(const std::string& filename, umap_tag_filters& filters);

    bool createDirectories(const std::string& path);
}

namespace html {
    // do not extract text from the content of these elements
    const std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} );

    // html elements that are self-closing (no content)
    const std::unordered_set<std::string> voidTags ( {"!doctype", "area", "base", "br",
        "col", "command", "embed", "hr", "img", "input", "keygen", "link", "meta",
        "param", "source", "track", "wbr"} );

    // block html elements
    // br is technically inline, but for the purposes of text extraction is should be treated as block
    const std::unordered_set<std::string> blockTags ( {"address", "article", "aside",
        "blockquote", "body", "br", "details", "dialog", "dd", "div", "dl", "dt",
        "fieldset", "figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4",
        "h5", "h6", "head", "header", "hgroup", "html", "hr", "li", "main", "nav",
        "ol", "p", "pre", "section", "table", "td", "th", "title", "tr", "ul"} );

    // inline html elements
    const std::unordered_set<std::string> inlineTags ( {"a", "abbr", "acronym", "audio",
        "b", "bdi", "bdo", "big", "button", "canvas", "cite", "code", "data",
        "datalist", "del", "dfn", "em", "embed", "i", "iframe", "img", "input",
        "ins", "kdb", "label", "map", "mark", "meter", "noscript", "object",
        "output", "picture", "progress", "q", "ruby", "s", "samp", "script",
        "select", "slot", "small", "span", "strong", "sub", "sup", "svg", "template",
        "textarea", "time", "u", "tt", "var", "video", "wbr" });

    inline bool isBlockTag(const std::string& tag) { return util::uset_contains(blockTags, tag); }
    inline bool isInlineTag(const std::string& tag) { return util::uset_contains(inlineTags, tag); }
    inline bool isVoidTag(const std::string& tag) { return util::uset_contains(voidTags, tag); }
    inline bool isNoTextTag(const std::string& tag) { return util::uset_contains(noText, tag); }
}

#endif
