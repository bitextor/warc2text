#include "deferred.hh"
#include <iostream>

namespace warc2text {

    DeferredTree::DeferredTree() : level(0), tag_stack(), counts() {
        std::unordered_map<std::string, unsigned int> umap;
        counts.push_back(umap);
    }

    void DeferredTree::insertTag(const std::string& tag) {
        tag_stack.push_back({tag, 0});
        counts.at(level)[tag]++;
        ++level;
        if (counts.size() < level+1) {
            std::unordered_map<std::string, unsigned int> umap;
            counts.push_back(umap);
        }
    }

    bool DeferredTree::empty() const {
        return tag_stack.empty();
    }

    void DeferredTree::addOffset(int n) {
        tag_stack.back().offset += n;
    }

    void DeferredTree::endTag() {
        tag_stack.pop_back();
        counts.at(level).clear();
        --level;
    }

    void DeferredTree::appendStandoff(std::string& deferred, int wordLength) const {
        for (unsigned int l = 0; l < tag_stack.size(); ++l) {
            deferred.append(tag_stack.at(l).tag);
            unsigned int i = counts.at(l).at(tag_stack.at(l).tag);
            if (i > 1) {
                deferred.push_back('[');
                deferred.append(std::to_string(i));
                deferred.push_back(']');
            }
            deferred.push_back('/');
        }
        deferred.back() = ':';
        deferred.append(std::to_string(tag_stack.back().offset));
        deferred.push_back('-');
        deferred.append(std::to_string(tag_stack.back().offset + wordLength - 1));
    }

} //warc2text
