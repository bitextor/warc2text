#ifndef WARC2TEXT_LANG_HH
#define WARC2TEXT_LANG_HH

#include <string>
#include <utility>
#include <vector>
#include "cld2/public/compact_lang_det.h"
#include "cld2/public/encodings.h"

namespace warc2text {
    struct LanguageDetection {
        std::string languageCode;
        int percent;
        double score;

        LanguageDetection(std::string lang, int percent, double score) :
            languageCode(std::move(lang)),
            percent(percent),
            score(score)
        {}

    };

    // detect language of plain text, return top 3 languages
    bool detectLanguage(const std::string& text, std::vector<LanguageDetection>& results);


    // detect top language of plain text
    bool detectLanguage(const std::string& text, std::string& lang);
}

#endif
