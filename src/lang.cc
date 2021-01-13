#include "src/lang.hh"

namespace warc2text {
    // hint = {content language code(s), tld, original encoding, CLD2::Language}
    const CLD2::CLDHints NO_HINT = {nullptr, nullptr, CLD2::UNKNOWN_ENCODING, CLD2::UNKNOWN_LANGUAGE};

    bool detectLanguage(const std::string& text, std::vector<LanguageDetection>& results){
        CLD2::Language top3[3] = {CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE};
        int percents[3] = {0,0,0};
        double scores[3] = {0.0, 0.0, 0.0};

        bool reliable = false;
        int text_bytes;
        int valid_prefix_bytes;

        CLD2::ResultChunkVector chunks;

        CLD2::ExtDetectLanguageSummaryCheckUTF8(text.data(), text.size(), true, &NO_HINT, 0, &top3[0], &percents[0], &scores[0], &chunks, &text_bytes, &reliable, &valid_prefix_bytes);

        results.clear();

        if (not reliable) return reliable;

        results.emplace_back(CLD2::LanguageCode(top3[0]), percents[0], scores[0]);
        if (percents[1] > 0)
            results.emplace_back(CLD2::LanguageCode(top3[1]), percents[1], scores[1]);
        if (percents[2] > 0)
            results.emplace_back(CLD2::LanguageCode(top3[2]), percents[2], scores[2]);

        if (results.size() == 1) return reliable;

        for (const CLD2::ResultChunk& chunk : chunks) {
            int index = chunk.lang1 == static_cast<CLD2::Language>(top3[0]) ? 0 :
                        chunk.lang1 == static_cast<CLD2::Language>(top3[1]) ? 1 : 2;
            results[index].chunks.emplace_back(chunk.offset, chunk.bytes);
        }

        return reliable;
    }

    bool detectLanguage(const std::string& text, std::string& lang){
        bool reliable = false;
        int valid_prefix_bytes = 0;
        CLD2::Language l = CLD2::DetectLanguageCheckUTF8(text.data(), text.size(), true, &reliable, &valid_prefix_bytes);
        lang = CLD2::LanguageCode(l);
        return reliable;
    }
} // namespace warc2text
