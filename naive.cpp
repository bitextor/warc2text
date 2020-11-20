// The longest common subsequence in C++

#include <iostream>
#include <vector>
#include <map>
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
        printf("Usage: Pass two file paths as command line arguments "
               "to compare the edit distance between them.\n");return 1;
    }

    std::ifstream t_doc_text(argv[1]);
    std::ifstream t_doc_url(argv[2]);
    std::string doc_text;
    std::string doc_url;
    std::unordered_map<std::string, std::vector<std::string>> m;
    while (std::getline(t_doc_url, doc_url))
    {
        std::getline(t_doc_text, doc_text);
        std::string decoded_doc_text;
        util::decodeBase64(doc_text,decoded_doc_text);
        std::istringstream ss(decoded_doc_text);
        std::string token;
        while(std::getline(ss, token)) {
            m[doc_url].push_back(token);
        }
    }

    std::ifstream t_sentences(argv[3]);
    std::string line_sent;
    std::vector<std::string> decoded_doc_lines;
    int pos_index;
    int block_index;
    std::string prev_url;
    std::string line_parts[5];

    while (std::getline(t_sentences, line_sent))
    {
        std::istringstream ss(line_sent);
        std::string token;
        int i = 0;
        while(std::getline(ss, token, '\t')) {
            line_parts[i] = token;
        }

        if (line_parts[0] != prev_url)
        {
            pos_index = 0;
            block_index = 0;
        }

        pos_index = naivePatternSearch(m[line_parts[0]][block_index].substr(pos_index), line_sent);

        prev_url = line_parts[0];
        if (pos_index != -1)
            std::cout << "Sentence '" << line_sent << "' found at block " << block_index << " at position: " << pos_index <<std::endl;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Time taken by Naive Pattern Searching algorithm: " << duration.count() << " microseconds" << std::endl;
}
