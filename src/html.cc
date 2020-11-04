#include "html.hh"
#include <deque>
#include "deferred.hh"

namespace warc2text {

    std::unordered_set<std::string> startNL ( {"ul", "ol", "dl", "tr"} );
    std::unordered_set<std::string> endNL ( {"p", "div", "li", "dd", "th", "td", "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9"} );
    std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} );
    std::unordered_set<std::string> voidTags ( {"!doctype", "area", "base", "br", "col", "command", "embed", "hr", "img", "input", "keygen", "link", "meta", "param", "source", "track", "wbr"} );

    inline bool isStartTag(const char* tag) { return startNL.find(util::toLowerCopy(tag)) != startNL.end(); }
    inline bool isEndTag(const char* tag) { return endNL.find(tag) != endNL.end(); }
    inline bool isNoText(const char* tag) { return noText.find(tag) != noText.end(); }
    inline bool isVoidTag(const char* tag) { return voidTags.find(util::toLowerCopy(tag)) != voidTags.end(); }

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

        DeferredTree dtree;

        while (t != markup::scanner::TT_EOF and t != markup::scanner::TT_ERROR) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
                    retval = util::HTML_PARSING_ERROR;
                case markup::scanner::TT_EOF:
                    break;
                case markup::scanner::TT_TAG_START:
                    if (isStartTag(sc.get_tag_name()))
                        plaintext.push_back('\n');
                    if (!isVoidTag(sc.get_tag_name())) {
                        dtree.insertTag(sc.get_tag_name());
                    }
                    break;
                case markup::scanner::TT_TAG_END:
                    if (!isVoidTag(sc.get_tag_name())) {
                        dtree.endTag();
                    }
                    if (isEndTag(sc.get_tag_name()) or isVoidTag(sc.get_tag_name()))
                        plaintext.push_back('\n');
                    else
                        plaintext.push_back(' ');
                    break;
                case markup::scanner::TT_WORD:
                    if (!dtree.empty()) {
                        dtree.appendStandoff(deferred, strlen(sc.get_value()));
                        deferred.push_back(';');
                        dtree.addOffset(strlen(sc.get_value()));
                    }
                    if (!isNoText(sc.get_tag_name()) and strcmp(sc.get_value(), "&nbsp;") != 0)
                            plaintext.append(sc.get_value());
                    break;
                case markup::scanner::TT_SPACE:
                    if (!dtree.empty())
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
