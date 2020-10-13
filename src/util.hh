#ifndef WARC2TEXT_UTIL_HH
#define WARC2TEXT_UTIL_HH

#include <string>

namespace util {
    // trim consecutive spaces but respect newlines
    void trimAllSpaces(std::string& text);

}

#endif 
