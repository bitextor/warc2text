#include "deferred.hh"
#include <iostream>

namespace warc2text {
    void endStandoffSegment(std::string& deferred) {
        if (!deferred.empty() and deferred.back() == '+')
            deferred.back() = ';';
        else if (!deferred.empty() and deferred.back() != ';')
            deferred.push_back(';');
    }

    void continueStandoffSegment(std::string& deferred) {
        if (!deferred.empty() and deferred.back() != ';' and deferred.back() != '+')
            deferred.push_back('+');
    }

    DeferredTree::DeferredTree() : DeferredTree(true) {};

    DeferredTree::DeferredTree(bool deferred) : deferred(deferred), level(0), tag_stack(), counts() {
        counts.emplace_back();
    }

    void DeferredTree::insertTag(const std::string& tag) {
        if (!deferred) return;
        tag_stack.push_back({tag, 0, 0});
        counts.at(level)[tag]++;
        ++level;
        if (counts.size() < level+1)
            counts.emplace_back();
    }

    bool DeferredTree::empty() const {
        return !deferred or tag_stack.empty();
    }

    void DeferredTree::addOffset(int n) {
        if (empty()) return;
        tag_stack.back().offset += n;
    }

    void DeferredTree::addLength(int n) {
        if (empty()) return;
        tag_stack.back().length += n;
    }

    void DeferredTree::endTag() {
        if (empty()) return;
        tag_stack.pop_back();
        counts.at(level).clear();
        --level;
    }

    unsigned int DeferredTree::getCurrentOffset() const {
        if (empty()) return 0;
        return tag_stack.back().offset;
    }

    unsigned int DeferredTree::getCurrentLength() const {
        if (empty()) return 0;
        return tag_stack.back().length;
    }

    void DeferredTree::setCurrentOffset(unsigned int n) {
        if (empty()) return;
        tag_stack.back().offset = n;
    }

    void DeferredTree::setCurrentLength(unsigned int n) {
        if (empty()) return;
        tag_stack.back().length = n;
    }

    void DeferredTree::appendAndOffset(std::string& deferred) {
        if (getCurrentLength() > 0) {
            appendStandoff(deferred);
            setCurrentOffset(getCurrentLength());
            setCurrentLength(0);
        }
    }

    void DeferredTree::appendStandoff(std::string& deferred) const {
        if (!this->deferred) return;
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
        if(!deferred.empty()) deferred.back() = ':';
        deferred.append(std::to_string(getCurrentOffset())); // start position
        deferred.push_back('-');
        deferred.append(std::to_string(getCurrentOffset() + getCurrentLength())); // length of segment
    }

} //warc2text
