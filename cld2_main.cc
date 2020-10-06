#include <iostream>
#include "cld2/public/compact_lang_det.h"

int main(int argv, char** argc){
    std::string text = "the quick brown fox jumps over the lazy dog";
    bool reliable = false;
    int valid_prefix_bytes = 0;
    CLD2::Language l = CLD2::DetectLanguageCheckUTF8(text.c_str(), text.size(), true, &reliable, &valid_prefix_bytes);
    std::cout << CLD2::LanguageName(l) << std::endl;
    return 0;
}
