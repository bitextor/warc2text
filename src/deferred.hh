#ifndef WARC2TEXT_DEFERRED_HH
#define WARC2TEXT_DEFERRED_HH

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

namespace warc2text {
    struct DeferredNode {
        std::string tag;
        unsigned int offset;
    };

    class DeferredTree {
        private:
            unsigned int level;
            std::deque<DeferredNode> tag_stack;
            std::vector<std::unordered_map<std::string, unsigned int>> counts;

        public:
            DeferredTree();
            void insertTag(const std::string& tag);
            void addOffset(int n);
            void endTag();
            void appendStandoff(std::string& s, int wordLength) const;
            bool empty() const;

    };
} //warc2text

#endif
