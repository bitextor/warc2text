#include "src/lang.hh"
#include "cld2/public/compact_lang_det.h"
#include "cld2/public/encodings.h"

namespace warc2text {
    // hint = {content language code(s), tld, original encoding, CLD2::Language}
    const CLD2::CLDHints NO_HINT = {nullptr, nullptr, CLD2::UNKNOWN_ENCODING, CLD2::UNKNOWN_LANGUAGE};

    CLD2Detector::~CLD2Detector() {}

    void CLD2Detector::detect(const std::string& text, std::unordered_map<std::string, std::string>& text_by_lang) const {
        bool reliable = false;
        int valid_prefix_bytes = 0;
        CLD2::Language l = CLD2::DetectLanguageCheckUTF8(text.data(), text.size(), true, &reliable, &valid_prefix_bytes);
        text_by_lang[reliable ? CLD2::LanguageCode(l) : kUnknownLanguageLabel] = text;
    }

    CLD2MultiLangDetector::~CLD2MultiLangDetector() {}

    void CLD2MultiLangDetector::detect(const std::string& text, std::unordered_map<std::string, std::string>& text_by_lang) const {
        CLD2::Language langs[3] = {CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE};
        int percents[3] = {0,0,0};
        double scores[3] = {0.0, 0.0, 0.0};

        bool reliable = false;
        int text_bytes;
        int valid_prefix_bytes;

        CLD2::ResultChunkVector chunks;

        CLD2::ExtDetectLanguageSummaryCheckUTF8(text.data(), text.size(), true, &NO_HINT, 0, &langs[0], &percents[0], &scores[0], &chunks, &text_bytes, &reliable, &valid_prefix_bytes);

        text_by_lang.clear();

        if (not reliable) {
            text_by_lang[kUnknownLanguageLabel] = text;
            return;
        }

        std::string* top1 = nullptr;
        std::string* top2 = nullptr;
        std::string* top3 = nullptr;

        if (langs[0] != CLD2::UNKNOWN_LANGUAGE and percents[0] > 0) {
            top1 = &text_by_lang[CLD2::LanguageCode(langs[0])];
            top1->reserve(text.size() * (percents[0] + 1));
        }

        if (langs[1] != CLD2::UNKNOWN_LANGUAGE and percents[1] > 0) {
            top2 = &text_by_lang[CLD2::LanguageCode(langs[1])];
            top2->reserve(text.size() * (percents[1] + 1));
        }

        if (langs[2] != CLD2::UNKNOWN_LANGUAGE and percents[2] > 0) {
            top3 = &text_by_lang[CLD2::LanguageCode(langs[2])];
            top3->reserve(text.size() * (percents[2] + 1));
        }

        for (const CLD2::ResultChunk& chunk : chunks) {
            std::string* ref = static_cast<CLD2::Language>(chunk.lang1) == langs[0] ? top1 :
                        static_cast<CLD2::Language>(chunk.lang1) == langs[1] ? top2 :
                        static_cast<CLD2::Language>(chunk.lang1) == langs[2] ? top3 : nullptr;
            if (ref == nullptr) continue;
            ref->append(text, chunk.offset, chunk.bytes);
        }

        // remove empty texts from text_by_lang
        // apparently it is possible that the reported percentage is > 0, but the language does not appear in chunks
        for (auto it = text_by_lang.cbegin(); it != text_by_lang.cend(); ){
            if (it->second.size() == 0) text_by_lang.erase(it++);
            else ++it;
        }

        // TODO: do something with the scores?
    }
} // namespace warc2text
