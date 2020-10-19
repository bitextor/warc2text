#include <iostream>
#include <string>
#include <vector>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "src/warcpreprocessor.hh"

using namespace warc2text;

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
        std::cerr << "Usage: " << argv[0] << " -o output_folder [WARC_files]" << std::endl;
        exit(1);
    }
    po::notify(vm);
}


int main(int argc, char *argv[]) {
    boost::log::add_console_log(std::cerr, boost::log::keywords::format = "[%TimeStamp%] [\%Severity%]: %Message%");
    boost::log::add_common_attributes();
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    // parse arguments
    Options options;
    parseArgs(argc,argv, options);

    WARCPreprocessor warcpproc(options.output);
    for (std::string file : options.warcs){
        warcpproc.process(file);
    }
    warcpproc.printStatistics();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    unsigned int hours = std::chrono::duration_cast<std::chrono::hours>(end - start).count();
    unsigned int minutes = std::chrono::duration_cast<std::chrono::minutes>(end - start).count() - hours*60;
    unsigned int seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count() - hours*60*60 - minutes*60;
    BOOST_LOG_TRIVIAL(info) << "elapsed: " << hours << "h" << minutes << "m" << seconds << "s";

    return 0;
}
