#include "src/lang.hh"
#include "cld2/public/compact_lang_det.h"
#include "cld2/public/encodings.h"

namespace warc2text {
    // hint = {content language code(s), tld, original encoding, CLD2::Language}
    const CLD2::CLDHints NO_HINT = {nullptr, nullptr, CLD2::UNKNOWN_ENCODING, CLD2::UNKNOWN_LANGUAGE};

    CLD2Detector::~CLD2Detector() {}

    void CLD2Detector::detect(AnnotatedText &&text, std::unordered_map<std::string, AnnotatedText>& text_by_lang) const {
        bool reliable = false;
        int valid_prefix_bytes = 0;
        CLD2::Language l = CLD2::DetectLanguageCheckUTF8(text.text.data(), text.text.size(), true, &reliable, &valid_prefix_bytes);
        text_by_lang[reliable ? CLD2::LanguageCode(l) : kUnknownLanguageLabel] = std::move(text);
    }

    CLD2MultiLangDetector::~CLD2MultiLangDetector() {}

    void CLD2MultiLangDetector::detect(AnnotatedText &&text, std::unordered_map<std::string, AnnotatedText>& text_by_lang) const {
        CLD2::Language langs[] = {CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE};
        int percents[3] = {0,0,0};
        double scores[3] = {0.0, 0.0, 0.0};

        bool reliable = false;
        int text_bytes;
        int valid_prefix_bytes;

        CLD2::ResultChunkVector chunks;

        CLD2::ExtDetectLanguageSummaryCheckUTF8(text.text.data(), text.text.size(), true, &NO_HINT, 0, &langs[0], &percents[0], &scores[0], &chunks, &text_bytes, &reliable, &valid_prefix_bytes);

        text_by_lang.clear();

        if (!reliable) {
            text_by_lang[kUnknownLanguageLabel] = std::move(text);
            return;
        }

        const char* mapping[] = {nullptr, nullptr, nullptr};

        for (size_t i = 0; i < 3; ++i) {
            if (langs[i] != CLD2::UNKNOWN_LANGUAGE && percents[i] > 0)
                mapping[i] = CLD2::LanguageCode(langs[2]);
        }

        for (const CLD2::ResultChunk& chunk : chunks) {
            if (chunk.bytes == 0) // TODO: can this even happen?
                continue;

            // Which of the top 3 languages is this chunk in?
            std::size_t i = 0;
            for (; i < 3; ++i) {
                if (static_cast<CLD2::Language>(chunk.lang1) == langs[i])
                    break;
            }

            // Chunk is not in top 3
            if (i == 3)
                continue;

            // Chunk is in top 3, append it to that AnnotatedText
            text_by_lang[mapping[i]].append(text, chunk.offset, chunk.bytes);
        }

        // TODO: do something with the scores?
    }
} // namespace warc2text
