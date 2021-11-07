#include "src/lang.hh"

#include "fasttext.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

namespace warc2text {

LanguageDetector::LanguageDetector(const std::string &filename)
  : classifier_(new fasttext::FastText) {
  classifier_->loadModel(filename);
}

LanguageDetector::~LanguageDetector() {}

const char kLabelPrefix[] = "__label__";

bool LanguageDetector::detect(const std::string& text, std::unordered_map<std::string, std::string>& text_by_lang) const {
  const float kThreshold = 0.5f;
  std::vector<std::pair<fasttext::real, std::string> > predictions;
  // TODO eliminate this copy by refactoring fastText
  std::stringstream stream(text);
  std::size_t begin_offset = 0;
  while (classifier_->predictLine(stream, predictions, 1, kThreshold)) {
    std::size_t end_offset = stream.tellg();
    if (!predictions.empty()) {
      // Labels look like __label__eng
      const std::string &label = predictions[0].second;
      if (strncmp(label.c_str(), kLabelPrefix, sizeof(kLabelPrefix) - 1)) {
        std::cerr << "Was expecting text classifier labels to begin with " << kLabelPrefix << " but they look like " << label << std::endl;
        std::abort();
      }
      std::string actual_label(label.data() + sizeof(kLabelPrefix) - 1, label.size() - (sizeof(kLabelPrefix) - 1));
      text_by_lang[actual_label].append(text.data() + begin_offset, text.data() + end_offset);
    }
    begin_offset = end_offset;
  }
  return !text_by_lang.empty();
}

} // namespace warc2text
