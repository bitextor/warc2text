#ifndef WARC2TEXT_UTIL_HH
#define WARC2TEXT_UTIL_HH

#include <string>

namespace util {
    // trim consecutive spaces but respect newlines
    void trimLines(std::string& text);
    void trimLinesCopy(const std::string& original, std::string& result);

}

#endif 
