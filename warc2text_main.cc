#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_set>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/algorithm/string/split.hpp>
#include <nlohmann/json.hpp>
#include "src/lang.hh"
#include "src/warcpreprocessor.hh"
#include "src/warcreader.hh"

using namespace warc2text;
using json_error = nlohmann::ordered_json::error_handler_t;

struct Options : WARCPreprocessorOptions {
    std::vector<std::string> warcs;
    std::string files;
    bool stdout{};
    bool verbose{};
    bool silent{};
    bool jsonl{};
    std::string classifier;
    std::string fasttext_model;
    std::string compress;
    int compress_level;
    std::string encoding_errors;
};

void parseArgs(int argc, char *argv[], Options& out) {
    namespace po = boost::program_options;
    po::options_description desc("Arguments");
    desc.add_options()
        ("help,h", po::bool_switch(), "Show this help message")
        ("output,o", po::value(&out.output)->default_value("."), "Output folder")
        ("stdout", po::bool_switch(&out.stdout)->default_value(false), "Write to standard output, only valid with --jsonl")
        ("files,f", po::value(&out.files)->default_value("url,text"), "List of output files separated by commas. Default: 'url,text'. Optional: 'mime,html,file'")
        ("input,i", po::value(&out.warcs)->multitoken(), "Input WARC file name(s)")
        ("tag-filters", po::value(&out.tag_filters_filename), "Plain text file containing tag filters")
        ("invert-tag-filters", po::bool_switch(&out.tag_filters_invert)->default_value(false), "Invert tag filter application")
        ("url-filters", po::value(&out.url_filters_filename), "Plain text file containing url filters")
        ("pdfpass", po::value(&out.pdf_warc_filename), "Write PDF records to WARC")
        ("robotspass", po::value(&out.robots_warc_filename), "Write robots.txt records to WARC")
        ("robots-process", po::bool_switch(&out.robots_process), "Process robots.txt as normal documents")
        ("paragraph-identification", po::bool_switch(&out.paragraph_identification)->default_value(false), "Add paragraph index in each b64encoded document as tab separated column")
        ("skip-text-extraction", po::bool_switch(&out.skip_text_extraction)->default_value(false))
        ("verbose,v", po::bool_switch(&out.verbose)->default_value(false), "Verbosity level")
        ("silent,s", po::bool_switch(&out.silent)->default_value(false))
        ("multilang", po::bool_switch(&out.multilang)->default_value(false), "Detect multiple languages in a single record")
        ("jsonl", po::bool_switch(&out.jsonl)->default_value(false), "Output in jsonl format")
        ("classifier", po::value(&out.classifier)->default_value("cld2"), "Language classifier: cld2 or fasttext (default cld2)")
        ("fasttext-model", po::value(&out.fasttext_model)->default_value(""), "Path to fasttext model")
        ("encode-urls", po::bool_switch(&out.encodeURLs)->default_value(false), "Encode URLs obtained from WARC records")
        ("compress", po::value(&out.compress)->default_value("gzip"), "Compression type for the output files")
        ("compress-level", po::value<int>(&out.compress_level)->default_value(3), "Compression level for the output files")
        ("encoding-errors", po::value(&out.encoding_errors)->default_value("replace"), "How encoding errors should be handled")
        ;

    po::positional_options_description pd;
    pd.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    if (argc == 1 || vm["help"].as<bool>()) {
        std::cerr << "Usage: " << argv[0] << " -o <output_folder> [ -f <output_files> ] [ --pdfpass <output_warc> ] [ --paragraph-identification ] [ --tag-filters <filters_file> ] <warc_file>...\n"
                "\n"
                "Options:\n"
                " -o <output_folder>               Output folder, required\n"
                " -f <output_files>                List of output files separated by commas\n"
                "                                  Default: \"url,text\"\n"
                "                                  Optional values: \"mime,html,file,date,metadata\"\n"
                " --classifier                     Classifier to use: cld2, fasttext or skip\n"
                " --fasttext-model <model_file>    Path to FastText model for fasttext classifier\n"
                " --multilang                      Detect multiple languages in documents (up to 3),\n"
                "                                  write as many text records as languages detected\n"
                " --tag-filters <filters_files>    File containing html tag filters\n"
                "                                  Format: \"html_tag <tab> tag_attr <tab> regexp\"\n"
                " --invert-tag-filters             Only output records that got filtered\n"
                " --url-filters <filters_file>     File containing url filters\n"
                "                                  Format: \"regexp\"\n"
                " --pdfpass <output_warc>          Write PDF records to <output_warc>\n"
                " --robotspass <output_warc>       Write Robots.txt records to <output_warc>\n"
                " --robots-process                 Process Robots.txt as any other document, instead of throwing them out\n"
                " --encode-urls                    Encode URLs obtained from WARC records\n"
                " --paragraph-identification       Add paragraph index for each sentence extracted from the html\n"
                " --skip-text-extraction           Skip text extraction and output only html\n"
                "                                  This option is not compatible with \"text\" value in -f option \n"
                "                                  and also requires to skip language identification\n"
                " --jsonl                          Produce \"html\" and \"text\" files in JSONLines format,\n"
                "                                  instead of bease64 encoded lines\n"
                " --stdout                         Write all the information in JSONLines to stdout\n"
                "                                  Needs --jsonl option\n"
                " --compress <compression>         Compression algorithm for the output files\n"
                "                                  Default: gzip. Values: gzip or zstd\n"
                " --compress-level <level>         Compression level to use\n"
                " --encoding-errors <handle>       How encoding errors should be handled\n"
                "                                  Possible values: ignore, replace (default), discard\n"
                "                                  discard will discard every document that contains errors\n"
                " -s                               Only output errors\n"
                " -v                               Verbose output (print trace)\n\n";
        exit(1);
    }
    po::notify(vm);
}


int main(int argc, char *argv[]) {
    // parse arguments
    Options options;
    parseArgs(argc,argv, options);

    // configure logging
    boost::log::add_console_log(std::cerr, boost::log::keywords::format = "[%TimeStamp%] [\%Severity%] %Message%");
    boost::log::add_common_attributes();
    auto verbosity_level = options.verbose ? boost::log::trivial::trace :
                           options.silent  ? boost::log::trivial::warning :
                                             boost::log::trivial::info;
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= verbosity_level);

    // prepare list of output files
    std::vector<std::string> files_list;
    boost::algorithm::split(files_list, options.files, [](char c) {return c == ',';});
    options.output_files.insert(files_list.begin(), files_list.end());

    if (options.robots_process && !options.robots_warc_filename.empty()) {
        BOOST_LOG_TRIVIAL(error) << "'--robotspass' and '--robots-process' are mutually exclusive.";
        abort();
    }
    if (options.skip_text_extraction) {
        if (options.files.find("text") != std::string::npos) {
            BOOST_LOG_TRIVIAL(error) << "Cannot use 'text' as output file with '--skip-text-extraction'. Please use '-f url,html' or any other combination that does not include it.";
            abort();
        }
        if (options.classifier != "skip") {
            BOOST_LOG_TRIVIAL(error) << "When skipping text extraction, language identification cannot be performed. Please provide '--classifier skip' to skip language identification.";
            abort();
        }
        if (options.tag_filters_filename != "")
            BOOST_LOG_TRIVIAL(warning) << "If '--skip-text-extraction' is enabled, tag filters cannot be applied.";
    }

    Compression compression;
    if (options.compress == "gzip") {
        compression = Compression::gzip;
    } else if (options.compress == "zstd") {
        compression = Compression::zstd;
    } else {
        BOOST_LOG_TRIVIAL(error) << "Invalid output compression type '" << options.compress << "'";
        abort();
    }

    json_error encoding_errors;
    if (options.encoding_errors == "ignore") {
        encoding_errors = json_error::ignore;
    } else if (options.encoding_errors == "replace") {
        encoding_errors = json_error::replace;
    } else if (options.encoding_errors == "discard") {
        encoding_errors = json_error::strict;
    } else {
        BOOST_LOG_TRIVIAL(error) << "Invalid encoding_errors value '" << options.encoding_errors << "'";
        abort();
    }

    std::unique_ptr<RecordWriter> writer;
    if (options.jsonl && options.stdout) {
        writer = std::make_unique<JSONLinesWriter>(std::cout, encoding_errors);
    } else if (!options.output_files.empty()) {
        Format format = Format::b64;
        if (options.jsonl)
            format = Format::json;
        writer = std::make_unique<BilangWriter>(options.output, options.output_files, compression, options.compress_level, format, encoding_errors);
    } else {
        BOOST_LOG_TRIVIAL(error) << "No output files specified";
        abort();
    }

    std::unique_ptr<LanguageDetector> detector;
    if (options.classifier == "cld2") {
        if (options.multilang) {
            detector.reset(new CLD2MultiLangDetector());
        } else {
            detector.reset(new CLD2Detector());
        }
    } else if (options.classifier == "fasttext") {
        if (options.multilang) {
            BOOST_LOG_TRIVIAL(error) << "FastText classifier doesn't do multilang at the moment";
            abort();
        } else if (options.fasttext_model.empty()) {
            BOOST_LOG_TRIVIAL(error) << "No FastText language identification model specified. Use --fasttext-model";
            abort();
        } else {
            detector.reset(new FastTextDetector(options.fasttext_model));
        }
    } else if (options.classifier == "skip") {
        if (options.multilang) {
            BOOST_LOG_TRIVIAL(error) << "Language identification is being skipped, ignoring --multilang option.";
        }
        detector.reset(new SkipLanguageDetector());
    } else {
        BOOST_LOG_TRIVIAL(error) << "Unsupported classifier option";
        abort();
    }

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    bool warc_file_error = false;
    try {
        WARCPreprocessor warcpproc(*writer, *detector, options);
        for (const std::string& file : options.warcs){
            try {
                warcpproc.process(file);
            } catch (const WARCFileException &e) {
                warc_file_error = true;
                continue;
            }
        }
        warcpproc.printStatistics();

    } catch (const std::exception &e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
        abort();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    if (warc_file_error)
        BOOST_LOG_TRIVIAL(error) << "There were WARC files that failed to open";

    unsigned int hours = std::chrono::duration_cast<std::chrono::hours>(end - start).count();
    unsigned int minutes = std::chrono::duration_cast<std::chrono::minutes>(end - start).count() - hours*60;
    unsigned int seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count() - hours*60*60 - minutes*60;
    BOOST_LOG_TRIVIAL(info) << "elapsed: " << hours << "h" << minutes << "m" << seconds << "s";

    return 0;
}
