#include "src/lang.hh"

#include "fasttext.h"
#include "util/exception.hh"

#include <cstdlib>
#include <cstring>

namespace warc2text {

FastTextDetector::FastTextDetector(const std::string &filename)
  : classifier_(new fasttext::FastText) {
  classifier_->loadModel(filename);
}

FastTextDetector::~FastTextDetector() {}

const char kLabelPrefix[] = "__label__";

void FastTextDetector::detect(AnnotatedText &&text, std::unordered_map<std::string, AnnotatedText>& chunks) const {
  const float kThreshold = 0.5f;
  std::vector<int32_t> words, labels;
  classifier_->getDictionary()->getStringNoNewline(text.text, words, labels);
  fasttext::Predictions predictions;
  classifier_->predict(1, words, predictions, kThreshold);
  if (predictions.empty()) {
    chunks[kUnknownLanguageLabel] = std::move(text);
    return;
  }

  // Labels look like __label__eng
  std::string label = classifier_->getDictionary()->getLabel(predictions[0].second);
  UTIL_THROW_IF2(strncmp(label.c_str(), kLabelPrefix, sizeof(kLabelPrefix) - 1), "Was expecting text classifier labels to begin with " << kLabelPrefix << " but they look like " << label);
  label.erase(0, sizeof(kLabelPrefix) - 1);

  // For better or worse, we're currently doing everything as one chunk.
  chunks[label] = std::move(text);
}

} // namespace warc2text
