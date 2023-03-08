#include "src/lang.hh"

#include "fasttext.h"

#include <cstdlib>
#include <cstring>

namespace warc2text {

FastTextDetector::FastTextDetector(const std::string &filename)
  : classifier_(new fasttext::FastText) {
  classifier_->loadModel(filename);
}

FastTextDetector::~FastTextDetector() {}

const char kLabelPrefix[] = "__label__";

bool FastTextDetector::detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const {
  const float kThreshold = 0.5f;
  std::vector<int32_t> words, labels;
  classifier_->getDictionary()->getStringNoNewline(text, words, labels);
  fasttext::Predictions predictions;
  classifier_->predict(1, words, predictions, kThreshold);
  if (predictions.empty()) return false;

  // Labels look like __label__eng
  std::string label = classifier_->getDictionary()->getLabel(predictions[0].second);
  if (strncmp(label.c_str(), kLabelPrefix, sizeof(kLabelPrefix) - 1)) {
    std::cerr << "Was expecting text classifier labels to begin with " << kLabelPrefix << " but they look like " << label << std::endl;
    std::abort();
  }
  label.erase(0, sizeof(kLabelPrefix) - 1);

  // For better or worse, we're currently doing everything as one chunk.
  chunks[label] = text;
  return true;
}

} // namespace warc2text
