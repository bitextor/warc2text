#include "util.hh"
#include <algorithm>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <uchardet/uchardet.h>

namespace util {
    void toLower(std::string& s){
        boost::algorithm::to_lower(s);
    }

    void trim(std::string& s){
        boost::algorithm::trim(s);
    }

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

    bool detectCharset(const std::string& text, std::string& charset){
        uchardet_t handle = uchardet_new();
        int chardet_result = uchardet_handle_data(handle, text.c_str(), text.size());
        uchardet_data_end(handle);
        bool success = (chardet_result == 0);
        if (success){
            charset = uchardet_get_charset(handle);
            toLower(charset);
        }
        uchardet_delete(handle);
        return (success && !charset.empty());
    }

    void encodeBase64(const std::string& original, std::string& base64){
        int pad = (3 - original.size() % 3) % 3;
        base64 = std::string(base64_text(original.begin()), base64_text(original.end()));
        base64.append(pad, '=');
    }

}
