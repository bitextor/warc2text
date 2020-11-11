// Copyright (c) 2017 Daniel Mabugat; 2020 Leopoldo Pla
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split
#include <sstream>
#include <fstream>
#include <chrono>
#include <iostream>


// Function declarations
uint32_t get_edit_distance(std::vector<std::string> words_1, std::vector<std::string> words_2);

int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    if (argc < 3) {
        printf("Usage: Pass two strings as command line arguments "
               "to compare the edit distance between them.\n");
        exit(1);
    }
    std::vector<std::string> words_1;
    std::vector<std::string> words_2;
    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();
    boost::split(words_1, buffer.str(), boost::is_any_of(" "), boost::token_compress_on);

    std::ifstream t_2(argv[2]);
    std::stringstream buffer_2;
    buffer_2 << t_2.rdbuf();
    boost::split(words_2, buffer_2.str(), boost::is_any_of(" "), boost::token_compress_on);

    uint32_t result = get_edit_distance(words_1, words_2);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    printf("Edit distance between '%s' and '%s' is %d\n", argv[1], argv[2], result);
    std::cout << "Time taken by function: "
         << duration.count() << " microseconds" << std::endl;
}


//Receives an input of two vectors of strings (words/tokens) and returns the Levenshtein distance between
//them as an unsigned integer. Function dynamically allocates a two dimensional
//array it later frees before the function returns.
uint32_t get_edit_distance(const std::vector<std::string> words_1, const std::vector<std::string> words_2) {
    uint32_t str_len_1 = words_1.size();
    uint32_t str_len_2 = words_2.size();
    uint32_t i, j;

    // Create data matrix
    uint32_t **data_mat = new uint32_t *[str_len_1 + 1];
    for (i = 0; i <= str_len_1; i += 1)
        data_mat[i] = new uint32_t[str_len_2 + 1];

    // Do work
    for (i = 0; i <= str_len_1; i += 1)
        for (j = 0; j <= str_len_2; j += 1) {
            if (i == 0)
                data_mat[i][j] = j;
            else if (j == 0)
                data_mat[i][j] = i;
            else if (words_1[i-1] == words_2[j-1])
                data_mat[i][j] = data_mat[i-1][j-1];
            else
                data_mat[i][j] = 1 + std::min(std::min(data_mat[i-1][j], data_mat[i][j-1]),data_mat[i-1][j-1]);
        }
    uint32_t result = data_mat[str_len_1][str_len_2];
    delete[] data_mat;
    return result;
}