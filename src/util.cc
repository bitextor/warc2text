#include "util.hh"
#include <algorithm>

namespace util {
    // TODO: right now this leaves a single space at the beginning and the end of the line
    void trimAllSpaces(std::string& text) {
        std::string::iterator new_end = std::unique(text.begin(), text.end(), [](char lhs, char rhs){return (lhs == '\n' && rhs == '\n') || (std::isspace(lhs) && rhs != '\n' && std::isspace(rhs));});
        text.erase(new_end, text.end());
    }
}
