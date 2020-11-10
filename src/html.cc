#include "html.hh"
#include "deferred.hh"
#include <deque>

namespace warc2text {

    std::unordered_set<std::string> startNL ( {"ul", "ol", "dl", "tr"} ); // put a new line BEFORE these elements
    std::unordered_set<std::string> endNL ( {"p", "div", "li", "dd", "th", "td", "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9"} ); // put a new line AFTER these elements

    std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} ); // do not extract text from the content of these elements

    // html elements that are self-closing (no content)
    std::unordered_set<std::string> voidTags ( {"!doctype", "area", "base", "br", "col", "command", "embed", "hr", "img", "input", "keygen", "link", "meta", "param", "source", "track", "wbr"} );

    // block html elements
    std::unordered_set<std::string> blockTags ( {"address", "article", "aside", "blockquote", "details", "dialog", "dd", "div", "dl", "dt", "fieldset", "figcaption", "figure", "footer", "form",
                                                 "h1", "h2", "h3", "h4", "h5", "h6", "header", "hgroup", "hr", "li", "main", "nav", "ol", "p", "pre", "section", "table", "ul"} );

    // inline html elements
    std::unordered_set<std::string> inlineTags ( {"a", "abbr", "acronym", "audio", "b", "bdi", "bdo", "big", "br", "button", "canvas", "cite", "code", "data", "datalist", "del", "dfn", "em", "embed",
                                                  "i", "iframe", "img", "input", "ins", "kdb", "label", "map", "mark", "meter", "noscript", "object", "output", "picture", "progress", "q", "ruby",
                                                  "s", "samp", "script", "select", "slot", "small", "span", "strong", "sub", "sup", "svg", "template", "textarea", "time", "u", "tt", "var", "video", "wbr" });

    inline bool startNewLine(const std::string& tag) { return startNL.find(tag) != startNL.end(); }
    inline bool endNewLine(const std::string& tag) { return endNL.find(tag) != endNL.end() or voidTags.find(tag) != voidTags.end(); }

    inline bool isNoText(const std::string& tag) { return noText.find(tag) != noText.end(); }
    inline bool isVoidTag(const std::string& tag) { return voidTags.find(tag) != voidTags.end(); }
    inline bool isInlineTag(const std::string& tag) { return inlineTags.find(tag) != inlineTags.end(); }
    inline bool isBlockTag(const std::string& tag) { return blockTags.find(tag) != blockTags.end(); }

    // true if doc is ok
    bool filter(markup::scanner& sc, const util::umap_tag_filters& tagFilters) {
        util::umap_tag_filters::const_iterator tag_it = tagFilters.find(sc.get_tag_name());
        if (tag_it == tagFilters.cend())
            return true;
        util::umap_attr_filters::const_iterator attr_it = tag_it->second.find(sc.get_attr_name());
        if (attr_it == tag_it->second.cend())
            return true;
        for (const std::string& filter : attr_it->second){
            if (strstr(sc.get_value(), filter.c_str())) {
                return false;
            }
        }
        return true;
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
                    tag = util::toLowerCopy(sc.get_tag_name()); // sc.get_tag_name() only changes value after a new tag is found
                    if (startNewLine(tag))
                        plaintext.push_back('\n');
                    if (!isVoidTag(tag))
                        dtree.insertTag(sc.get_tag_name());
                    if (isBlockTag(tag) and !deferred.empty() and deferred.back() != ';')
                        deferred.push_back(';'); // found block tag: previous word has ended
                    break;
                case markup::scanner::TT_TAG_END:
                    tag = util::toLowerCopy(sc.get_tag_name()); // sc.get_tag_name() only changes value after a new tag is found
                    if (!isVoidTag(tag))
                        dtree.endTag();
                    if (endNewLine(tag))
                        plaintext.push_back('\n');
                    else
                        plaintext.push_back(' ');
                    break;
                case markup::scanner::TT_WORD:
                    // if the tag is is noText list, don't save the text or the standoff
                    if (isNoText(tag))
                        break;
                    plaintext.append(sc.get_value());
                    if (!deferred.empty() && deferred.back() != ';')
                        deferred.push_back('+');
                    dtree.appendStandoff(deferred, strlen(sc.get_value()));
                    dtree.addOffset(strlen(sc.get_value()));
                    break;
                case markup::scanner::TT_SPACE:
                    if (!deferred.empty() && deferred.back() != ';')
                        deferred.push_back(';'); // found space: previous word has ended
                    dtree.addOffset(strlen(sc.get_value()));
                    plaintext.push_back(' ');
                    break;
                case markup::scanner::TT_ATTR:
                    if (!filter(sc, tagFilters))
                        retval = util::FILTERED_DOCUMENT_ERROR;
                    break;
                default:
                    break;
            }
        }
        return retval;
    }

    void unescapeEntities(const std::string& plaintext, std::string& decoded) {
        char* decodedplaintext = new char [plaintext.size() + 1];
        decode_html_entities_utf8(decodedplaintext, plaintext.data());
        decoded = decodedplaintext;
        delete[] (decodedplaintext);
    }

}
