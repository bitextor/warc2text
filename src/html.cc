#include <cstring>
#include <unordered_set>
#include "util.hh"
#include "html.hh"
#include "deferred.hh"
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
            dtree.addLength(1); // comment this line if we don't want to count spaces
        }
    }

    int processHTML(const std::string& html, std::string& plaintext, bool extractStandoff, std::string& deferred, const util::umap_tag_filters& tagFilters){
        plaintext = "";
        markup::instream si(html.c_str());
        markup::scanner sc(si);

        int t = markup::scanner::TT_SPACE; // just start somewhere that isn't ERROR or EOF
        int retval = util::SUCCESS;
        std::string tag;

        DeferredTree dtree(extractStandoff);

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
                    dtree.appendAndOffset(deferred);
                    if (html::isBlockTag(tag)) {
                        // found block tag: previous block has ended
                        addNewLine(plaintext, dtree);
                        dtree.endStandoffSegment(deferred);
                    } else {
                        dtree.continueStandoffSegment(deferred);
                    }
                    if (t == markup::scanner::TT_TAG_START)
                        dtree.insertTag(tag);
                    else if (t == markup::scanner::TT_TAG_END)
                        dtree.endTag(tag);
                    break;
                case markup::scanner::TT_WORD:
                    // if the tag is in noText list, don't save the text or the standoff
                    if (html::isNoTextTag(tag))
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

}
