#include "html.hh"

namespace warc2text {

    std::unordered_set<std::string> startNL ( {"ul", "ol", "dl", "tr"} );
    std::unordered_set<std::string> endNL ( {"p", "div", "li", "dd", "th", "td", "h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9"} );
    std::unordered_set<std::string> selfNL ( {"br"} );
    std::unordered_set<std::string> noText ( {"script", "noscript", "style", ""} );

    void processHTML(const std::string& html, std::string& plaintext){
        plaintext = "";
        markup::instream si(html.c_str());
        markup::scanner sc(si);
        const char *value;
        int t = markup::scanner::TT_SPACE;
        while (t != markup::scanner::TT_EOF && t != markup::scanner::TT_ERROR) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
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
                case markup::scanner::TT_ATTR:
                    break;
                case markup::scanner::TT_WORD:
                    if (noText.find(sc.get_tag_name()) == noText.end()) {
                        value = sc.get_value();
                        if (strcmp(value,"&nbsp;") != 0) {
                            plaintext.append(value);
                        }
                    }
                    break;
                case markup::scanner::TT_SPACE:
                    plaintext.push_back(' ');
                    break;
                default:
                    break;
            }
        }
    }


    void unescapeEntities(const std::string& plaintext, std::string& decoded) {
        char* decodedplaintext = new char [plaintext.size() + 1];
        decode_html_entities_utf8(decodedplaintext, plaintext.data());
        decoded = decodedplaintext;
        delete[] (decodedplaintext);
    }

}
