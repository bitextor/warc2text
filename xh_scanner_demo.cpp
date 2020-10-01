// xh_scanner_demo.cpp : Defines the entry point for the console application.
//

#include <cstdio>

#include "xh_scanner.h"
#define __unused __attribute__((unused))

struct str_istream : public markup::instream {
    const char *p;
    const char *end;

    explicit str_istream(const char *src) : p(src), end(src + strlen(src)) {}

    wchar_t get_char() override { return p < end ? *p++ : 0; }
};


int main(__unused int argc, __unused char *argv[]) {
    str_istream si(
            "<html><body><p align=right dir='rtl'>Begin &amp; back</p>"
            "<a href=http://terrainformatica.com/index.php?a=1&b=2>link</a></body></html>");
    markup::scanner sc(si);
    while (true) {
        int t = sc.get_token();
        switch (t) {
            case markup::scanner::TT_ERROR:
                printf("ERROR\n");
                break;
            case markup::scanner::TT_EOF:
                printf("EOF\n");
                goto FINISH;
            case markup::scanner::TT_TAG_START:
                printf("TAG START:%s\n", sc.get_tag_name());
                break;
            case markup::scanner::TT_TAG_END:
                printf("TAG END:%s\n", sc.get_tag_name());
                break;
            case markup::scanner::TT_ATTR:
                printf("\tATTR:%s=%S\n", sc.get_attr_name(), sc.get_value());
                break;
            case markup::scanner::TT_WORD:
            case markup::scanner::TT_SPACE:
                printf("{%S}\n", sc.get_value());
                break;
            default:
                printf("Unknown tag\n");
                break;
        }
    }
    FINISH:
    printf("--------------------------\n");
    return 0;
}


