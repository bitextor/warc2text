#include "util.hh"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/locale.hpp>
#include <boost/log/trivial.hpp>
#include <uchardet/uchardet.h>
#include "preprocess/base64.hh"

namespace util {
    void toLower(std::string& s){
        boost::algorithm::to_lower(s);
    }
    std::string toLowerCopy(const std::string& s){
        return boost::algorithm::to_lower_copy(s);
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
        result.clear();
        result.reserve(original.size()); // Worst case

        auto text_begin = original.begin();
        while (text_begin != original.end()) {
            // Find begin of text (trim whitespace at the front of line)
            if (std::isspace(*text_begin)) {
                ++text_begin;
                continue;
            }

            // Find line ending
            auto line_end = std::find(text_begin, original.end(), '\n');

            // Find where, before the line ending, the actual text ended
            auto text_end = line_end;
            while (text_end != text_begin && std::isspace(*(text_end - 1)))
                --text_end;

            // If there was text between text_begin and text_end, result!
            if (text_end != text_begin) {// not empty line
                result.append(text_begin, text_end);
                result.append("\n");
            }
            
            // Jump to next line
            // (std::is_space will take care of \n if there is any text left)
            text_begin = line_end;
        }
    }

    bool detectCharset(const std::string& text, std::string& charset, const std::string& original_charset){
        uchardet_t handle = uchardet_new();
        int chardet_result = uchardet_handle_data(handle, text.c_str(), text.size());
        uchardet_data_end(handle);
        bool success = (chardet_result == 0);
        // trust the detected more than the specified charset
        if (success){
            charset = uchardet_get_charset(handle);
            toLower(charset);
        } else {
            // if detection fails, go with the original one
            charset = toLowerCopy(original_charset);
        }
        uchardet_delete(handle);
        if (charset.empty()) return false;

        // check that boost can work with the detected charset
        try {
            boost::locale::conv::to_utf<char>("", charset);
        } catch (const boost::locale::conv::invalid_charset_error& e) {
            return false;
        }
        return true;
    }

    std::string toUTF8(const std::string& text, const std::string& charset) {
        return boost::locale::conv::to_utf<char>(text, charset);
    }
    std::string toUTF8(const char* text, const std::string& charset) {
        return boost::locale::conv::to_utf<char>(text, charset);
    }

    void encodeBase64(const std::string& original, std::string& base64){
        preprocess::base64_encode(original, base64);
    }

    void decodeBase64(const std::string& base64, std::string& output){
        preprocess::base64_decode(base64, output);
    }

    void readTagFiltersRegex(const std::string& filename, umap_tag_filters_regex& filters) {
        std::ifstream f(filename);
        std::string line;
        std::vector<std::string> fields;
        for (size_t line_i=1; std::getline(f, line); ++line_i) {
            if (boost::algorithm::all(line, boost::algorithm::is_space()) || boost::algorithm::starts_with(line, "#"))
                continue;
            fields.clear();
            boost::algorithm::split(fields, line, [](char c){return c == '\t';});
            if (fields.size() < 3) {
                BOOST_LOG_TRIVIAL(warning) << "Could not parse tag filter at line " << line_i << " of " << filename;
                continue;
            }
            umap_attr_filters_regex& attrs = filters[fields.at(0)];
            std::vector<umap_attr_regex>& values = attrs[fields.at(1)];
            for (unsigned int i = 2; i < fields.size(); ++i)
                values.emplace_back((umap_attr_regex){
                    std::regex(fields.at(i), std::regex::optimize | std::regex::nosubs),
                    fields.at(i)
                });
        }
        f.close();
    }

    void readUrlFiltersRegex(const std::string &filename, boost::regex &urlFilter) {
        std::ifstream f(filename);
        std::string line;
        std::ostringstream combined;
        bool first = true;
        for (size_t line_i=1; std::getline(f, line); ++line_i) {
            if (boost::algorithm::all(line, boost::algorithm::is_space()) || boost::algorithm::starts_with(line, "#"))
                continue;
            try {
                (boost::regex(line)); // Compile, but just to test its validity.
                if (first)
                    first = false;
                else
                    combined << "|";
                combined << "(" << line << ")";
            } catch (const boost::regex_error& e) {
                BOOST_LOG_TRIVIAL(warning) << "Could not parse url filter at " << filename << ":" << line_i << ": " << e.what();
                continue;
            }
        }
        f.close();

        urlFilter.assign(combined.str(), boost::regex::optimize | boost::regex::nosubs);
    }

    bool createDirectories(const std::string& path){
        if (!boost::filesystem::exists(path))
            return boost::filesystem::create_directories(path);
        else return false; // throw exception??
    }

    std::string encodeURLs(const std::string& in) {
        std::ostringstream out;
        out << std::hex;

        for (char c : in){
            // allowed characters
            if (std::isalnum(c) or c == '-' or c == '.' or c == '~') {
                out << c;
            }

            // characters with a reserved purpose
            // should be percent-encoded when used as data within a URI, but how do we tell?
            else if (reserved_chars_url.find(c) != std::string::npos){
                out << c;
            }

            // characters that should always be escaped
            else {
                out << "%" << int(c);
            }
        }
        return out.str();
    }

    std::vector<std::string> split(const std::string& s, const std::string& delimiter)
    {
        std::vector<std::string> result;
        size_t pos = 0, previous_pos = 0;
        std::string token;

        while ((pos = s.find(delimiter, previous_pos)) != std::string::npos) {
            result.push_back(s.substr(previous_pos, pos - previous_pos));

            previous_pos = pos + delimiter.size();
        }

        result.push_back(s.substr(previous_pos, s.size() - previous_pos));

        return result;
    }

}
