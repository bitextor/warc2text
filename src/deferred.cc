#include "deferred.hh"
#include <iostream>

namespace warc2text {
    DeferredNode *DeferredTree::newNode(const std::string& tag, DeferredNode* parent) {
        DeferredNode *n = new DeferredNode;
        n->tag = tag;
        n->parent = parent;
        n->offset = 0;
        return n;
    }

    void DeferredTree::deleteNode(DeferredNode* node){
        if (!node->children.empty())
            deleteChildren(node);
        node->parent = NULL;
        delete node;
    }

    void DeferredTree::deleteChildren(DeferredNode* node){
        while (!node->children.empty()){
            deleteNode(node->children.back());
            node->children.pop_back();
        }
    }

    DeferredTree::DeferredTree() {
        root = newNode("", NULL);
        current = root;
        level = 0;
    }

    DeferredTree::~DeferredTree() {
        current = NULL;
        deleteNode(root);
    }

    void DeferredTree::insertTag(const std::string& tag) {
        current->children.push_back(newNode(tag, current));
        current->counts[tag]++;
        current = current->children.back();
        ++level;
    }

    void DeferredTree::addOffset(int n) {
        current->offset += n;
    }

    void DeferredTree::endTag() {
        deleteChildren(current);
        current = current->parent;
        --level;
    }

    void DeferredTree::appendStandoff(std::string& deferred, int wordLength) const {
        if (root == NULL or root->children.empty()) return;
        const DeferredNode* node = root->children.back();
        while (true) {
            deferred.append(node->tag);
            if (node->parent != NULL and node->parent->counts.at(node->tag) > 1) {
                deferred.push_back('[');
                deferred.append(std::to_string(node->parent->counts.at(node->tag)));
                deferred.push_back(']');
            }
            if (node->children.empty()) break;
            deferred.push_back('/');
            node = node->children.back();
        }
        deferred.push_back(':');
        deferred.append(std::to_string(node->offset));
        deferred.push_back('-');
        deferred.append(std::to_string(node->offset + wordLength - 1));
    }

    bool DeferredTree::empty() const {
        return root == NULL;
    }

    void DeferredTree::printPreorder(const DeferredNode* node, int level) {
        for (int i = 0; i < level; ++i) std::cout << " ";
        std::cout << node->tag << "\n";
        for (const DeferredNode* child : node->children) {
            printPreorder(child, level+1);
        }
    }

    void DeferredTree::printTree() const {
        // root is always empty, so -1
        printPreorder(root, -1);
    }

} //warc2text
