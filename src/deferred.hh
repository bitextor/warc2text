#ifndef WARC2TEXT_DEFERRED_HH
#define WARC2TEXT_DEFERRED_HH

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

namespace warc2text {
    void endStandoffSegment(std::string& deferred);
    void continueStandoffSegment(std::string& deferred);

    struct DeferredNode {
        std::string tag;
        unsigned int offset;
        unsigned int length;
    };

    class DeferredTree {
        private:
            bool deferred;
            unsigned int level;
            std::deque<DeferredNode> tag_stack;
            std::vector<std::unordered_map<std::string, unsigned int>> counts;

        public:
            DeferredTree();
            DeferredTree(bool deferred);
            void insertTag(const std::string& tag);
            void addOffset(int n);
            void addLength(int n);
            void endTag();
            void appendAndOffset(std::string& s);
            void appendStandoff(std::string& s) const;
            unsigned int getCurrentOffset() const;
            unsigned int getCurrentLength() const;
            void setCurrentOffset(unsigned int n);
            void setCurrentLength(unsigned int n);
            bool empty() const;

    };
} //warc2text

#endif
