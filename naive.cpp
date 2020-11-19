// The longest common subsequence in C++

#include <iostream>
#include <fstream>
#include <chrono>

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
        printf("Usage: Pass two strings as command line arguments "
               "to compare the edit distance between them.\n");return 1;
    }

    std::ifstream t(argv[1]);
    std::string data((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>()); // used to store text data


    std::ifstream t_2(argv[2]);
    std::string data_2((std::istreambuf_iterator<char>(t_2)),
                       std::istreambuf_iterator<char>()); // used to store text data

    int index = naivePatternSearch(data, data_2);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Pattern found at position: " << index <<std::endl;

    std::cout << "Time taken by Naive Pattern Searching algorithm: " << duration.count() << " microseconds" << std::endl;
}
