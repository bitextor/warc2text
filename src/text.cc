#include "text.hh"
#include <cassert>

namespace {
    std::size_t count(char needle, std::string const &haystack, std::size_t offset = 0, std::size_t count = std::string::npos) {
        std::size_t hits = 0;

        size_t end;

        if (count == std::string::npos || offset + count > haystack.size())
            end = haystack.size();
       else
            end = offset + count;

        while (true) {
            std::size_t hit = haystack.find(needle, offset);
            if (hit >= end)
                break;

            ++hits;
            offset = hit + 1;
        }

        return hits;
    }

    template <typename T>
    void append(std::vector<T> &dest, std::vector<T> const &source, std::size_t offset, std::size_t count) {
        dest.reserve(dest.size() + count);
        for (std::size_t i = 0; i < count; ++i)
            dest.emplace_back(source[offset + i]);
    }
}

namespace warc2text {

AnnotatedText AnnotatedText::substr(std::size_t offset, std::size_t length) const {
    return AnnotatedText().append(*this, offset, length);
}

AnnotatedText &AnnotatedText::append(AnnotatedText const &other, std::size_t offset, std::size_t length) {
    std::size_t tag_offset = ::count('\n', other.text, 0, offset);
    std::size_t tag_length = ::count('\n', other.text, offset, length);

    // When the current text does not end with a newline, we skip copying the
    // first tag of `other` because that line will be added to the current line
    // that already has a tag.
    if (!text.empty() && text.back() != '\n') {
        tag_offset += 1;
        tag_length -= 1;
    }

    text.append(other.text, offset, length);
    ::append(tags, other.tags, tag_offset, tag_length);
    return *this;
}

void AnnotatedText::push_back(std::string const &chunk, std::string const &tag) {
    if (chunk.empty())
        return;

    std::size_t lines = 0;

    if (std::isspace(static_cast<unsigned char>(chunk.back()))) {
        text.append(chunk, 0, chunk.size() - 1);
        text.push_back('\n');
        lines = count('\n', chunk, 0, chunk.size() - 1) + 1;
    } else {
        text.reserve(text.size() + chunk.size() + 1);
        text += chunk;
        text.push_back('\n');
        lines = count('\n', chunk) + 1;
    }

    assert(lines >= 1);

    for (std::size_t i = 0; i < lines; ++i)
        tags.push_back(tag);
}

void AnnotatedText::clear() {
    text.clear();
    tags.clear();
}   

};
