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
    virtual bool detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const = 0;
};

class FastTextDetector : public LanguageDetector {
  public:
    explicit FastTextDetector(const std::string &filename);

    virtual ~FastTextDetector();

    // detect language of plain text, return top languages
    virtual bool detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;

  private:
    std::unique_ptr<fasttext::FastText> classifier_;
};

class CLD2Detector : public LanguageDetector {
public:
  virtual bool detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;
  virtual ~CLD2Detector();
};

class CLD2MultiLangDetector : public LanguageDetector {
public:
  virtual bool detect(const std::string& text, std::unordered_map<std::string, std::string>& chunks) const;
  virtual ~CLD2MultiLangDetector();
};

} // namespace warc2text

#endif
