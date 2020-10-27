#include "html.hh"
#include "util.hh"
#include <unordered_map>

namespace warc2text {

    struct str_istream : public markup::instream {
        const char *p;
        const char *end;

        explicit str_istream(const char *src) : p(src), end(src + strlen(src)) {}

        char get_char() override { return p < end ? *p++ : 0; }
    };

    std::unordered_set<std::string> startNL ( {"ul", "ol", "dl", "tr"} );
    std::unordered_set<std::string> endNL ( {"p", "div", "li", "dd", "th", "td", "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9"} );
    std::unordered_set<std::string> selfNL ( {"br"} );
    std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} );

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> filterTags ({
        {"link", { {"rel", "alternate machine-translated-from"}, {"id", "gtranslate-style-css"} }},
        {"meta", { {"name", "translation-stats"} } },
        {"aside", { {"id", "gtranslate"} } },
        {"script", { {"source", "gtranslate.net"} } },
    });

    int processHTML(const std::string& html, std::string& plaintext){
        plaintext = "";
        str_istream si(html.c_str());
        markup::scanner sc(si);

        auto tag_it = filterTags.cbegin();
        auto attr_it = tag_it->second.cbegin();
        int t = markup::scanner::TT_SPACE;
        int retval = util::SUCCESS;
        while (t != markup::scanner::TT_EOF && t != markup::scanner::TT_ERROR) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
                    retval = util::HTML_PARSING_ERROR;
                case markup::scanner::TT_EOF:
                    break;
                case markup::scanner::TT_TAG_START:
                    if (startNL.find(sc.get_tag_name()) != startNL.end()) {
                        plaintext.push_back('\n');
                    }
                    break;
                case markup::scanner::TT_TAG_END:
                    if (endNL.find(sc.get_tag_name()) != endNL.end() or selfNL.find(sc.get_tag_name()) != selfNL.end()) {
                        plaintext.push_back('\n');
                    } else {
                        plaintext.push_back(' ');
                    }
                    break;
                case markup::scanner::TT_WORD:
                    if (noText.find(sc.get_tag_name()) == noText.end()) {
                        if (strcmp(sc.get_value(),"&nbsp;") != 0) {
                            plaintext.append(sc.get_value());
                        }
                    }
                    break;
                case markup::scanner::TT_SPACE:
                    plaintext.push_back(' ');
                    break;
                case markup::scanner::TT_ATTR:
                    tag_it = filterTags.find(sc.get_tag_name());
                    if (tag_it != filterTags.end()) {
                        attr_it = tag_it->second.find(sc.get_attr_name());
                        if (attr_it != tag_it->second.cend()
                            && strstr(sc.get_value(), attr_it->second.c_str())) {
                            retval = util::FILTERED_DOCUMENT_ERROR;
                        }
                    }
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
