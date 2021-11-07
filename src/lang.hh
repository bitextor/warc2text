#ifndef WARC2TEXT_LANG_HH
#define WARC2TEXT_LANG_HH

#include <memory>
#include <string>
#include <unordered_map>

namespace fasttext {
class FastText;
} // namespace fasttext

namespace warc2text {

class LanguageDetector {
  public:
    explicit LanguageDetector(const std::string &filename);

    ~LanguageDetector();

    // detect language of plain text, return top languages
    bool detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;

  private:
    std::unique_ptr<fasttext::FastText> classifier_;
};

} // namespace warc2text

#endif
