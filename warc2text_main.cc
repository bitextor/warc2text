#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>

#include "src/record.hh"
#include "src/warcreader.hh"
#include "src/bilangwriter.hh"

using namespace warc2text;

void PreProcessFile(const std::string &filename, const std::string &folder) {
    BilangWriter writer(folder);
    WARCReader reader(filename);
    std::string content;
    auto start = std::chrono::steady_clock::now();
    auto end = start;
    auto warc_reading = start - end;
    auto record_parsing = warc_reading;
    auto html_cleaning = warc_reading;
    auto lang_detection = warc_reading;
    auto record_writing = warc_reading;

    bool done = !reader.getRecord(content);
    std::string plaintext;
    while (!done) {
        start = std::chrono::steady_clock::now();
        Record record = Record(content);
        end = std::chrono::steady_clock::now();
        record_parsing += (end - start);
        if (record.getHeaderProperty("WARC-Type") == "response") {
            start = std::chrono::steady_clock::now();
            record.cleanPayload();
            plaintext = record.getPlainText();
            end = std::chrono::steady_clock::now();
            html_cleaning += (end - start);
            if (!plaintext.empty()) {
                start = std::chrono::steady_clock::now();
                record.detectLanguage();
                end = std::chrono::steady_clock::now();
                lang_detection += (end - start);

                start = std::chrono::steady_clock::now();
                writer.write(record);
                end = std::chrono::steady_clock::now();
                record_writing += (end - start);
            }
        }
        start = std::chrono::steady_clock::now();
        done = !reader.getRecord(content);
        end = std::chrono::steady_clock::now();
        warc_reading += (end - start);
    }
    std::cerr << "warc reading: " << std::chrono::duration_cast<std::chrono::seconds>(warc_reading).count() << "s\n";
    std::cerr << "record parsing: " << std::chrono::duration_cast<std::chrono::seconds>(record_parsing).count() << "s\n";
    std::cerr << "html cleaning: " << std::chrono::duration_cast<std::chrono::seconds>(html_cleaning).count() << "s\n";
    std::cerr << "lang detection: " << std::chrono::duration_cast<std::chrono::seconds>(lang_detection).count() << "s\n";
    std::cerr << "record writing: " << std::chrono::duration_cast<std::chrono::seconds>(record_writing).count() << "s\n";
}

struct Options {
    std::vector<std::string> warcs;
    std::string output;
};

void parseArgs(int argc, char *argv[], Options& out) {
    namespace po = boost::program_options;
    po::options_description desc("Arguments");
    desc.add_options()
        ("help,h", po::bool_switch(), "Show this help message")
        ("output,o", po::value(&out.output)->default_value("."), "Output folder")
        ("input,i", po::value(&out.warcs)->multitoken(), "Input WARC file name(s)");

    po::positional_options_description pd;
    pd.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    if (argc == 1 || vm["help"].as<bool>()) {
        std::cerr << "you forgot the arguments you dumm-dumm!\n";
        exit(1);
    }
    po::notify(vm);
}


int main(int argc, char *argv[]) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    // parse arguments
    Options options;
    parseArgs(argc,argv, options);

    for (std::string file : options.warcs){
        PreProcessFile(file, options.output);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    unsigned int elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

//    printf("total records: %d\n", proc.TotalRecords)
//    printf("text records: %d\n", proc.TextRecords)
//    printf("lang records: %d\n", proc.LangRecords)
//    printf("total bytes: %d\n", proc.TotalBytes)
//    printf("text bytes: %d\n", proc.TextBytes)
//    printf("lang bytes: %d\n", proc.LangBytes)
    std::cerr << "elapsed time: " << elapsed << "s\n";

    return 0;
}
