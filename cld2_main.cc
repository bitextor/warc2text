#include <iostream>
#include "cld2/public/compact_lang_det.h"
#include "cld2/public/encodings.h"

int main(int argv, char** argc){

    // input
    std::string text = "the quick brown fox jumps over the lazy dog. съешь ещё этих мягких французких булок, да выпей же чаю. le gustaba cenar un exquisito sándwich de jamón con zumo de piña y vodka frío";
    CLD2::CLDHints hints = {nullptr, nullptr, CLD2::UNKNOWN_ENCODING, CLD2::UNKNOWN_LANGUAGE};

    // output
    CLD2::Language top3[3] = {CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE, CLD2::UNKNOWN_LANGUAGE};
    int percent3[3] = {0, 0, 0};
    double scores3[3] = {0.0, 0.0, 0.0};
    CLD2::ResultChunkVector chunks;
    bool reliable = false;
    int text_bytes;
    int valid_prefix_bytes = 0;

    CLD2::Language l = CLD2::ExtDetectLanguageSummaryCheckUTF8(text.data(), text.size(), true, &hints, 0, &top3[0], &percent3[0], &scores3[0], &chunks, &text_bytes, &reliable, &valid_prefix_bytes);
    // std::cout << CLD2::LanguageName(l) << " " << CLD2::LanguageCode(l) << std::endl;

    std::cout << "n\tlang\tpercent\tscore\n";
    for (int i = 0; i < 3; ++i)
        std::cout << i << "\t" << CLD2::LanguageCode(top3[i]) << "\t" << percent3[i] << "\t" << scores3[i] << "\n";

    for (auto chunk : chunks)
        std::cout << CLD2::LanguageCode( (CLD2::Language) chunk.lang1) << " " << text.substr(chunk.offset, chunk.bytes) << std::endl; // substr makes a copy, don't use it in production

    return 0;
}
