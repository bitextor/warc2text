#include "html.hh"
#include "deferred.hh"

namespace warc2text {

    std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} ); // do not extract text from the content of these elements

    // html elements that are self-closing (no content)
    std::unordered_set<std::string> voidTags ( {"!doctype", "area", "base", "br", "col", "command", "embed", "hr", "img", "input", "keygen", "link", "meta", "param", "source", "track", "wbr"} );

    // block html elements
    std::unordered_set<std::string> blockTags ( {"address", "article", "aside", "blockquote", "body", "details", "dialog", "dd", "div", "dl", "dt", "fieldset", "figcaption", "figure", "footer", "form",
                                                 "h1", "h2", "h3", "h4", "h5", "h6", "head", "header", "hgroup", "html", "hr", "li", "main", "nav", "ol", "p", "pre", "section", "table", "td", "th", "title", "tr", "ul"} );

    // inline html elements
    std::unordered_set<std::string> inlineTags ( {"a", "abbr", "acronym", "audio", "b", "bdi", "bdo", "big", "br", "button", "canvas", "cite", "code", "data", "datalist", "del", "dfn", "em", "embed",
                                                  "i", "iframe", "img", "input", "ins", "kdb", "label", "map", "mark", "meter", "noscript", "object", "output", "picture", "progress", "q", "ruby",
                                                  "s", "samp", "script", "select", "slot", "small", "span", "strong", "sub", "sup", "svg", "template", "textarea", "time", "u", "tt", "var", "video", "wbr" });

    inline bool isNoText(const std::string& tag) { return noText.find(tag) != noText.end(); }
    inline bool isVoidTag(const std::string& tag) { return voidTags.find(tag) != voidTags.end(); }
    inline bool isInlineTag(const std::string& tag) { return inlineTags.find(tag) != inlineTags.end(); }
    inline bool isBlockTag(const std::string& tag) { return blockTags.find(tag) != blockTags.end(); }

    // true if doc is ok
    bool filter(const std::string& lc_tag, const char* attr, const char* value, const util::umap_tag_filters& tagFilters) {
        util::umap_tag_filters::const_iterator tag_it = tagFilters.find(lc_tag);
        if (tag_it == tagFilters.cend())
            return true;
        util::umap_attr_filters::const_iterator attr_it = tag_it->second.find(util::toLowerCopy(attr));
        if (attr_it == tag_it->second.cend())
            return true;
        std::string lc_value = util::toLowerCopy(value);
        for (const std::string& filter : attr_it->second){
            if (lc_value.find(filter) != std::string::npos)
                return false;
        }
        return true;
    }

    void addNewLine(std::string& plaintext, DeferredTree& dtree) {
        if (std::isspace(plaintext.back())) {
            plaintext.back() = '\n';
            if (dtree.getCurrentLength() > 0)
                dtree.addLength(-1);
        } else if (!plaintext.empty()) {
            plaintext.push_back('\n');
        }
    }

    void addSpace(std::string& plaintext, DeferredTree& dtree) {
        if (!plaintext.empty() && !std::isspace(plaintext.back())) {
            plaintext.push_back(' ');
            dtree.addLength(1);
        }
    }

    int processHTML(const std::string& html, std::string& plaintext, std::string& deferred, const util::umap_tag_filters& tagFilters){
        plaintext = "";
        markup::instream si(html.c_str());
        markup::scanner sc(si);

        int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF
        int retval = util::SUCCESS;
        std::string tag;

        DeferredTree dtree;

        while (t != markup::scanner::TT_EOF and t != markup::scanner::TT_ERROR) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
                    retval = util::HTML_PARSING_ERROR;
                case markup::scanner::TT_EOF:
                    break;
                case markup::scanner::TT_TAG_START:
                case markup::scanner::TT_TAG_END:
                    tag = util::toLowerCopy(sc.get_tag_name()); // sc.get_tag_name() only changes value after a new tag is found
                    dtree.appendAndOffset(deferred);
                    if (isBlockTag(tag)) {
                        // found block tag: previous block has ended
                        addNewLine(plaintext, dtree);
                        endStandoffSegment(deferred);
                    } else {
                        continueStandoffSegment(deferred);
                    }
                    if (!isVoidTag(tag)) {
                        if (t == markup::scanner::TT_TAG_START) dtree.insertTag(tag);
                        else if (t == markup::scanner::TT_TAG_END) dtree.endTag();
                    }
                    break;
                case markup::scanner::TT_WORD:
                    // if the tag is is noText list, don't save the text or the standoff
                    if (isNoText(tag))
                        break;
                    plaintext.append(sc.get_value());
                    dtree.addLength(strlen(sc.get_value()));
                    break;
                case markup::scanner::TT_SPACE:
                    addSpace(plaintext, dtree);
                    break;
                case markup::scanner::TT_ATTR:
                    if (!filter(tag, sc.get_attr_name(), sc.get_value(), tagFilters))
                        retval = util::FILTERED_DOCUMENT_ERROR;
                    break;
                default:
                    break;
            }
        }
        while (deferred.back() == '+' or deferred.back() == ';') deferred.pop_back();
        if (plaintext.back() != '\n') plaintext.push_back('\n');
        return retval;
    }

    void unescapeEntities(const std::string& plaintext, std::string& decoded) {
        char* decodedplaintext = new char [plaintext.size() + 1];
        decode_html_entities_utf8(decodedplaintext, plaintext.data());
        decoded = decodedplaintext;
        delete[] (decodedplaintext);
    }

}
