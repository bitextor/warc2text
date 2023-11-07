#pragma once

#include <string>
#include <vector>

namespace warc2text {
    class AnnotatedText {
    public:
        std::string text;
        std::vector<std::string> tags;

        // Extract a bit of text + tags from this text
        AnnotatedText substr(std::size_t offset, std::size_t count) const;

        AnnotatedText &append(AnnotatedText const &other, std::size_t offset, std::size_t count);

        // Append a block of text (and tag) at the end
        void push_back(std::string const &text, std::string const &tag);

        void clear();
    };
};
