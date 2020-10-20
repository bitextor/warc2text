#ifndef WARC2TEXT_UTIL_HH
#define WARC2TEXT_UTIL_HH

#include <string>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace util {
    void toLower(std::string& s);

    // trim consecutive spaces from left and right
    void trim(std::string& s);

    // trim consecutive spaces but respect newlines:
    void trimLines(std::string& text);
    void trimLinesCopy(const std::string& original, std::string& result);

    typedef boost::archive::iterators::base64_from_binary<
        boost::archive::iterators::transform_width<
            std::string::const_iterator,
            6,
            8
        >
    > base64_text;

    void encodeBase64(const std::string& original, std::string& base64);

}

#endif 
