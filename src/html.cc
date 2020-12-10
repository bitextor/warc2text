#include <cstring>
#include <unordered_set>
#include "util.hh"
#include "html.hh"
#include "xh_scanner.hh"

namespace warc2text {

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
            if (strstr(value, filter.c_str()) != NULL)
                return false;
        }
        return true;
    }

    void addNewLine(std::string& plaintext) {
        if (std::isspace(plaintext.back())) {
            plaintext.back() = '\n';
        } else if (!plaintext.empty()) {
            plaintext.push_back('\n');
        }
    }

    void addSpace(std::string& plaintext) {
        if (!plaintext.empty() && !std::isspace(plaintext.back())) {
            plaintext.push_back(' ');
        }
    }

    int processHTML(const std::string& html, std::string& plaintext, const util::umap_tag_filters& tagFilters){
        plaintext = "";
        markup::instream si(html.c_str());
        markup::scanner sc(si);

        int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF
        int retval = util::SUCCESS;
        std::string tag;

        while (t != markup::scanner::TT_EOF and t != markup::scanner::TT_ERROR) {
            t = sc.get_token();
            switch (t) {
                case markup::scanner::TT_ERROR:
                    retval = util::HTML_PARSING_ERROR;
                case markup::scanner::TT_EOF:
                    break;
                case markup::scanner::TT_TAG_START:
                case markup::scanner::TT_TAG_END:
                    // sc.get_tag_name() only changes value after a new tag is found
                    tag = util::toLowerCopy(sc.get_tag_name());
                    // found block tag: previous block has ended
                    if (html::isBlockTag(tag)) addNewLine(plaintext);
                    break;
                case markup::scanner::TT_WORD:
                    // if the tag is in noText list, don't save the text
                    if (html::isNoTextTag(tag)) break;
                    plaintext.append(sc.get_value());
                    break;
                case markup::scanner::TT_SPACE:
                    addSpace(plaintext);
                    break;
                case markup::scanner::TT_ATTR:
                    if (!filter(tag, sc.get_attr_name(), sc.get_value(), tagFilters))
                        retval = util::FILTERED_DOCUMENT_ERROR;
                    break;
                default:
                    break;
            }
        }
        if (plaintext.back() != '\n') plaintext.push_back('\n');
        return retval;
    }

}
