#ifndef WARC2TEXT_HTML_HH
#define WARC2TEXT_HTML_HH

#include <unordered_set>
#include <cstring>
#include <string>
#include "xh_scanner.hh"
#include "util.hh"
extern "C" {
    #include "entities.h"
}

namespace warc2text {
    int processHTML(const std::string& html, std::string& text, const util::umap_tag_filters& tagFilters);

    void unescapeEntities(const std::string& text, std::string& processed);
}

#endif
