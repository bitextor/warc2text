#ifndef WARC2TEXT_DEFERRED_HH
#define WARC2TEXT_DEFERRED_HH

#include <string>
#include <deque>
#include <unordered_map>

namespace warc2text {
    struct DeferredNode {
        std::string tag;
        DeferredNode* parent;
        int offset;
        std::unordered_map<std::string, int> counts;
        std::deque<DeferredNode*> children;
    };

    class DeferredTree {
        private:
            int level;
            DeferredNode *root;
            DeferredNode *current;
            static DeferredNode *newNode(const std::string& tag, DeferredNode* parent);
            static void deleteNode(DeferredNode* node);
            static void deleteChildren(DeferredNode* node);
            static void printPreorder(const DeferredNode* node, int level);

        public:
            DeferredTree();
            ~DeferredTree();
            void insertTag(const std::string& tag);
            void addOffset(int n);
            void endTag();
            void appendStandoff(std::string& s, int wordLength) const;
            bool empty() const;
            void printTree() const;
    };
} //warc2text

#endif
