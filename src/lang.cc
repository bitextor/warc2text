#include "lang.hh"

namespace warc2text {
	
const std::string LanguageDetector::kUnknownLanguageLabel = "unk";


SkipLanguageDetector::~SkipLanguageDetector() {}

void SkipLanguageDetector::detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const {
    // When skipping language detection, you may think "unk" should be the label used
    // but this may seem as langid tried but only found unks.
    // So leaving it as empty for now
    chunks[""] = text;
}

} // namespace warc2text
