#include "util.hh"
#include <algorithm>
#include <boost/algorithm/string/trim_all.hpp>

namespace util {
    // TODO: right now this leaves a single space at the beginning and the end of the line
    void trimLines(std::string& text) {
        std::string::iterator new_end = std::unique(text.begin(), text.end(), [](char lhs, char rhs){return (lhs == '\n' && rhs == '\n') || (std::isspace(lhs) && rhs != '\n' && std::isspace(rhs));});
        text.erase(new_end, text.end());
    }

    void trimLinesCopy(const std::string& original, std::string& result){
        result = "";
        auto first = original.begin();
        auto last = std::find(original.begin(), original.end(), '\n');
        std::string line = "";
        while (first < original.end()) {
            line = std::string(first, last);
            boost::trim_all(line);
            if (!line.empty()){
                result.append(line);
                result.append("\n");
            }
            first = last + 1;
            last = std::find(first, original.end(), '\n');
        }
    }
}
