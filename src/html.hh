#ifndef WARC2TEXT_HTML_HH
#define WARC2TEXT_HTML_HH

#include <unordered_set>
#include <string.h>
#include "xh_scanner.hh"
extern "C" {
    #include "entities.h"
}

namespace warc2text {
    void processHTML(const std::string& html, std::string& text);

    void unescapeEntities(const std::string& text, std::string& processed);
}

#endif
