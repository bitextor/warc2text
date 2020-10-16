#ifndef WARC2TEXT_UTIL_HH
#define WARC2TEXT_UTIL_HH

#include <string>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace util {
    // trim consecutive spaces but respect newlines
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
