#include <iostream>
#include <vector>
#include <unordered_set>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/algorithm/string/split.hpp>
#include "src/warcpreprocessor.hh"

using namespace warc2text;

struct Options {
    std::vector<std::string> warcs;
    bool verbose{};
    bool silent{};
};

void parseArgs(int argc, char *argv[], Options& out, WARCPreprocOptions& warc_out) {
    namespace po = boost::program_options;
    po::options_description desc("Arguments");
    std::string files;
    desc.add_options()
        ("help,h", po::bool_switch(), "Show this help message")
        ("output,o", po::value(&warc_out.output_folder)->default_value("."), "Output folder")
        ("files,f", po::value(&files)->default_value("url,token"), "List of output files separated by commas. Default (mandatory files): 'url,text'. Optional: 'mime,html'")
        ("input,i", po::value(&out.warcs)->multitoken(), "Input WARC file name(s)")
        ("tag-filters", po::value(&warc_out.tag_filters_file), "Plain text file containing tag filters")
        ("invert-tag-filters", po::bool_switch(&warc_out.tag_filters_invert)->default_value(false), "Invert tag filter application")
        ("pdfpass", po::value(&warc_out.pdf_warc_filename), "Write PDF records to WARC")
        ("verbose,v", po::bool_switch(&out.verbose)->default_value(false), "Verbosity level")
        ("silent,s", po::bool_switch(&out.silent)->default_value(false))
        ("multilang", po::bool_switch(&warc_out.multilang)->default_value(false), "Detect multiple languages in a single record")
        ("pdfextract", po::value(&warc_out.pdfextract_jar)->default_value(""), "path to PDFExtract-2.0.jar")
        ("pdf-config", po::value(&warc_out.pdfextract_config_file)->default_value(""), "path to PDFExtract.json")
        ("pdf-log", po::value(&warc_out.pdfextract_log_file)->default_value(""), "write PDFExtract log to file");

    po::positional_options_description pd;
    pd.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    if (argc == 1 || vm["help"].as<bool>()) {
        std::cerr << "Usage: " << argv[0] << " -o <output_folder> [ -f <output_files> ] [ --pdfpass <output_warc> ] [ --tag-filters <filters_file> ] <warc_file>...\n"
                "\n"
                "Options:\n"
                " -o <output_folder>               Output folder, required\n"
                " -f <output_files>                List of output files separated by commas\n"
                "                                  Default (mandatory): \"url,text\"\n"
                "                                  Optional values: \"mime,html\"\n"
                " --multilang                      Detect multiple languages in documents (up to 3),\n"
                "                                  write as many text records as languages detected\n"
                " --tag-filters <filters_files>    File containing filters\n"
                "                                  Format: \"html_tag <tab> tag_attr <tab> regexp\"\n"
                " --invert-tag-filters             Only output records that got filtered\n"
                " --pdfpass <output_warc>          Write PDF records to <output_warc>\n"
                " --pdfextact <jar_file>           Path to PDFExtract-2.0.jar\n"
                " --pdf-config <config_file>       Path to PDFExtract.json config file\n"
                " --pdf-log <log_file>             Write PDFExtract log to this file\n"
                " -s                               Only output errors\n"
                " -v                               Verbose output (print trace)\n\n";
        exit(1);
    }
    po::notify(vm);

    // prepare list of output files
    std::vector<std::string> files_list;
    boost::algorithm::split(files_list, files, [](char c) {return c == ',';});
    warc_out.files.insert(files_list.begin(), files_list.end());
}


int main(int argc, char *argv[]) {
    // parse arguments
    Options options;
    WARCPreprocOptions warc_options;
    parseArgs(argc,argv, options, warc_options);

    // configure logging
    boost::log::add_console_log(std::cerr, boost::log::keywords::format = "[%TimeStamp%] [\%Severity%] %Message%");
    boost::log::add_common_attributes();
    auto verbosity_level = options.verbose ? boost::log::trivial::trace :
                           options.silent  ? boost::log::trivial::warning :
                                             boost::log::trivial::info;
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= verbosity_level);

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    WARCPreprocessor warcpproc(warc_options);
    for (const std::string& file : options.warcs){
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
