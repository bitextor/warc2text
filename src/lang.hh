#ifndef WARC2TEXT_LANG_HH
#define WARC2TEXT_LANG_HH

#include <string>
#include <unordered_map>
#include <utility>
#include "cld2/public/compact_lang_det.h"
#include "cld2/public/encodings.h"

namespace warc2text {
    // detect language of plain text, return top 3 languages
    bool detectLanguage(const std::string& text, std::unordered_map<std::string, std::string>& chunks);

    // detect top language of plain text
    bool detectLanguage(const std::string& text, std::string& lang);
}

#endif
