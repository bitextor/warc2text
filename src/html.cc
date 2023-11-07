#include <cstring>
#include <unordered_set>
#include <regex>
#include <boost/log/trivial.hpp>
#include "util.hh"
#include "html.hh"
#include "entities.hh"
#include "xh_scanner.hh"

namespace warc2text {

    // true if doc is ok
    bool filter(const std::string& lc_tag, const char* attr, const char* value, const util::umap_tag_filters_regex& tagFilters) {
        util::umap_tag_filters_regex::const_iterator tag_it = tagFilters.find(lc_tag);
        if (tag_it == tagFilters.cend())
            return true;
        util::umap_attr_filters_regex::const_iterator attr_it = tag_it->second.find(util::toLowerCopy(attr));
        if (attr_it == tag_it->second.cend())
            return true;
        for (const util::umap_attr_regex& filter : attr_it->second){
            if (std::regex_search(value, filter.regex)) {
                BOOST_LOG_TRIVIAL(debug) << "Tag filter " << tag_it->first << "[" << attr_it->first << " ~ " << filter.str << "] matched '" << value << "'";
                return false;
            }
        }
        return true;
    }

    void addSpace(std::string& plaintext) {
        if (!plaintext.empty() && !std::isspace(static_cast<unsigned char>(plaintext.back()))) {
            plaintext.push_back(' ');
        }
    }

    bool isWhitespace(std::string const &str) {
        return std::all_of(str.begin(), str.end(), [](unsigned char c){ return std::isspace(c); });
    }

    int processHTML(const std::string& html, AnnotatedText& plaintext, const util::umap_tag_filters_regex& tagFilters){
        plaintext.clear();

        markup::instream si(html.c_str());
        markup::scanner sc(si);

        int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF
        int retval = util::SUCCESS;
        std::string tag;
        std::string paragraph;
        std::string plain;

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
                    if (html::isBlockTag(tag) && !isWhitespace(paragraph)) {
                        // TODO: add this directly to the scanner?
                        entities::decodeEntities(paragraph, plain);
                        plaintext.push_back(plain, tag);
                        paragraph.clear(); // reset for next paragraph
                    }
                    // found void tag, like <img> or <embed>
                    if (html::isVoidTag(tag)) addSpace(paragraph);
                    break;
                case markup::scanner::TT_WORD:
                    // if the tag is in noText list, don't save the text
                    if (html::isNoTextTag(tag)) break;
                    paragraph.append(sc.get_value());
                    break;
                case markup::scanner::TT_SPACE:
                    addSpace(paragraph);
                    break;
                case markup::scanner::TT_ATTR:
                    if (!filter(tag, sc.get_attr_name(), sc.get_value(), tagFilters))
                        retval = util::FILTERED_DOCUMENT_ERROR;
                    break;
                default:
                    break;
            }
        }

        if (!isWhitespace(paragraph)) {
            entities::decodeEntities(paragraph, plain);
            plaintext.push_back(plain, "");
        }

        return retval;
    }

}
