// The Naive search algorithm in C++

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <fstream>
#include <sstream>
#include <chrono>
#include "src/util.hh"

int naivePatternSearch(std::string mainString, std::string pattern) {
    int patLen = pattern.size();
    int strLen = mainString.size();

    for(int i = 0; i<=(strLen - patLen); i++) {
        int j;
        for(j = 0; j<patLen; j++) {      //check for each character of pattern if it is matched
            if(mainString[i+j] != pattern[j])
                break;
        }

        if(j == patLen) {     //the pattern is found
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    if (argc < 3) {
        printf("Usage: Pass plain_text.gz, url.gz and deferred.gz from warc2text for two languages (so 6 files)"
               "to match their content with the lines from a sentence aligner-tab separated file"
               "given as last argument. This will generate a standoff annotation for those sentences or TU\n");return 1;
    }

    std::ifstream t_doc_text(argv[1]);
    std::ifstream t_doc_url(argv[2]);
    std::ifstream t_doc_standoff(argv[3]);
    std::string doc_text;
    std::string doc_url;
    std::string doc_standoff;
    std::unordered_map<std::string, std::vector<std::pair<std::string,std::string>>> source_blocks;
    while (std::getline(t_doc_url, doc_url))
    {
        std::getline(t_doc_text, doc_text);

        std::string decoded_doc_text;
        util::decodeBase64(doc_text,decoded_doc_text);

        std::getline(t_doc_standoff, doc_standoff);

        std::istringstream ss(decoded_doc_text);
        std::string token;

        std::istringstream sss(doc_standoff);
        std::string standoff;

        while(std::getline(ss, token)) {
            std::getline(sss, standoff, ';');
            std::pair<std::string,std::string> PAIR1;
            PAIR1.first = token;
            PAIR1.second = standoff;
            source_blocks[doc_url].push_back(PAIR1);
        }
    }

    std::ifstream t_sentences(argv[4]);
    std::string line_sent;
    std::vector<std::string> decoded_doc_lines;
    int pos_index_sl = 0;
    int block_index_sl = 0;
    std::string prev_url_sl;
    int pos_index_tl = 0;
    int block_index_tl = 0;
    std::string prev_url_tl;
    std::string line_parts[5];

    while (std::getline(t_sentences, line_sent))
    {
        std::istringstream ss(line_sent);
        std::string token;
        int i = 0;
        while(std::getline(ss, token, '\t')) {
            line_parts[i] = token;
            i++;
        }

        if (line_parts[0] != prev_url_sl)
        {
            pos_index_sl = 0;
            block_index_sl = 0;
        }
        if (line_parts[1] != prev_url_tl)
        {
            pos_index_tl = 0;
            block_index_tl = 0;
        }

        pos_index_sl = naivePatternSearch(source_blocks[line_parts[0]][block_index_sl].first.substr(pos_index_sl), line_parts[2]);
        pos_index_tl = naivePatternSearch(source_blocks[line_parts[1]][block_index_tl].first.substr(pos_index_tl), line_parts[3]);

        prev_url_sl = line_parts[0];
        prev_url_tl = line_parts[1];

        if (pos_index_sl != -1)
            std::cout << "SL Sentence '" << line_parts[2] << "' found at block " << block_index_sl << " at position: " << pos_index_sl << std::endl;
        else
            std::cout << "SL Sentence could not be found (maybe a concatenated string from sentence aligner)" << std::endl;

        if (pos_index_tl != -1)
            std::cout << "TL Sentence '" << line_parts[3] << "' found at block " << block_index_tl << " at position: " << pos_index_tl << std::endl;
        else
            std::cout << "TL Sentence could not be found (maybe a concatenated string from sentence aligner)" << std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Time taken by Naive Pattern Searching algorithm: " << duration.count() << " microseconds" << std::endl;
}
