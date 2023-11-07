#ifndef WARC2TEXT_HTML_HH
#define WARC2TEXT_HTML_HH

#include <string>
#include "text.hh"
#include "util.hh"

namespace warc2text {
    int processHTML(const std::string& html, AnnotatedText &text, const util::umap_tag_filters_regex& tagFilters);
}

#endif
