// The longest common subsequence in C++

#include <iostream>
#include <fstream>
#include <chrono>

void lcsAlgo(const std::string &S1, const std::string &S2, int m, int n, std::string &lcs) {
    int dp_table[m + 1][n + 1]; // Dynamic programming table

    // Building the matrix in bottom-up way
    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                dp_table[i][j] = 0;
            else if (S1[i - 1] == S2[j - 1])
                dp_table[i][j] = dp_table[i - 1][j - 1] + 1;
            else
                dp_table[i][j] = std::max(dp_table[i - 1][j], dp_table[i][j - 1]);
        }
    }

    // Recover longest common subsequence
    int index = dp_table[m][n];
    lcs.resize(index + 1);
    lcs[index] = '\0';

    int i = m, j = n;
    while (i > 0 && j > 0) {
        if (S1[i - 1] == S2[j - 1]) {
            lcs[index - 1] = S1[i - 1];
            i--;
            j--;
            index--;
        } else if (dp_table[i - 1][j] > dp_table[i][j - 1])
            i--;
        else
            j--;
    }
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

    std::string result;

    lcsAlgo(data, data_2, data.size(), data_2.size(), result);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "LCS is: " << result << std::endl;
    std::cout << "Time taken by LCS: " << duration.count() << " microseconds" << std::endl;
}
