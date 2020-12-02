/*  Copyright 2012 Christoph Gärtner
    Distributed under the Boost Software License, Version 1.0
*/

#ifndef DECODE_HTML_ENTITIES_UTF8_
#define DECODE_HTML_ENTITIES_UTF8_

#include <cstddef>
#include <unordered_map>

namespace entities {
    void decodeEntities(const std::string& source, std::string& target);
    std::string get_dec_entity(unsigned long cp);

    extern std::unordered_map<std::string, std::string> named_entities;
}

#endif
