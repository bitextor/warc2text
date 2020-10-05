#include <iostream>
#include "src/warcreader.hh"

int main(int argc, char* argv[]){
    std::string filename = "-";
    if (argc > 1 and argv[1] != "-")
        filename = std::string(argv[1]);

    WARCReader wr(filename);
    std::string content;
    while(wr.getRecord(content)){
        std::cout << "=== record starts ===" << std::endl;
        std::cout << content;
        std::cout << "=== record ends ===" << std::endl;
        content.clear();
    }

    return 0;
}
