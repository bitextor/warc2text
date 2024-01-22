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
    virtual ~LanguageDetector() {};

    // detect language of plain text, return top languages
    virtual void detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const = 0;

    // Label used for text (chunks) that cannot reliably be identified
    static const std::string kUnknownLanguageLabel;
};

class FastTextDetector : public LanguageDetector {
  public:
    explicit FastTextDetector(const std::string &filename);
    virtual ~FastTextDetector();
    virtual void detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;

  private:
    std::unique_ptr<fasttext::FastText> classifier_;
};

class CLD2Detector : public LanguageDetector {
public:
  virtual void detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;
  virtual ~CLD2Detector();
};

class CLD2MultiLangDetector : public LanguageDetector {
public:
  virtual void detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;
  virtual ~CLD2MultiLangDetector();
};

class SkipLanguageDetector : public LanguageDetector {
public:
  virtual void detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;
  virtual ~SkipLanguageDetector();
};

} // namespace warc2text

#endif
