#ifndef WARC2TEXT_HTML_HH
#define WARC2TEXT_HTML_HH

#include <string>

namespace warc2text {
    int processHTML(const std::string& html, std::string& text, const util::umap_tag_filters& tagFilters);
}

#endif
