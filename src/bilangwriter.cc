#include "bilangwriter.hh"
#include "util.hh"
#include "util/exception.hh"
#include <cassert>
#include <string>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <boost/log/trivial.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>


namespace warc2text {
    using json = nlohmann::ordered_json;

    CompressWriter::CompressWriter()
    : file(),
      compressor() {
          compression = Compression::gzip;
          level = 3;
    }

    CompressWriter::CompressWriter(Compression c, int l)
    : file(),
      compressor() {
          compression = c;
          level = l;
    }

    CompressWriter::~CompressWriter() {
        if (file.is_open()){
            compressor.reset();
        }
    }

    void CompressWriter::open(const std::string &filename) {
        file = std::ofstream(filename, std::ios_base::out | std::ios_base::binary);
        switch(compression) {
            case Compression::gzip:
                compressor.push(bio::gzip_compressor(bio::gzip_params(level)));
                break;
            case Compression::zstd:
                compressor.push(bio::zstd_compressor(bio::zstd_params(level)));
                break;
        }
        compressor.push(file);
    }

    void CompressWriter::writeLine(const std::string &text) {
        // creating an ostream on each document write seems inefficient
        // but ostream objects cannot be copyassigned and at the moment of construction
        // we don't have the underlying compressor, so cannot declare in constructor
        // and then assign to a new instance when in CompressWriter::open
        std::ostream(&compressor) << text << "\n";
    }

    bool CompressWriter::is_open() {
        return file.is_open();
    }

    json toJSON(Record const &record, std::string const &chunk, bool metadata_only) {
        json obj = {
             {"f", record.getFilename()},
             {"o", record.getOffset()},
             {"s", record.getSize()},
             {"rs", record.getPayload().size()},
             {"u", record.getURL()},
             {"c", record.getHTTPcontentType()},
             {"ts", record.getWARCdate()},
        };

        // Insert extracted plain text if requested
        if(!metadata_only) {
            obj["ps"] = chunk.size();
            obj["p"] = chunk;
        }

        return obj;
    }

    std::string toJSON(const std::string &text, const std::string &field_name,
            json::error_handler_t encoding_error) {
        json object = { {field_name, text} };
        std::string s;
        return object.dump(-1, ' ', false, encoding_error);
    }

    LangWriter::LangWriter(const std::string& path, const std::unordered_set<std::string>& output_files,
                           Compression c, int l, Format f, json::error_handler_t e)
    : metadata_file(c,l), url_file(c,l), mime_file(c,l), text_file(c,l), html_file(c,l), file_file(c,l), date_file(c,l), format(f), encoding_error(e)
    {
        util::createDirectories(path);

        std::string suffix;
        switch(c) {
            case Compression::zstd: suffix = ".zst"; break;
            case Compression::gzip: suffix = ".gz"; break;
        }

        if (output_files.count("metadata"))
            metadata_file.open(path + "/metadata" + suffix);
        if (output_files.count("url"))
            url_file.open(path + "/url" + suffix);
        if (output_files.count("text"))
            text_file.open(path + "/text" + suffix);
        if (output_files.count("mime"))
            mime_file.open(path + "/mime" + suffix);
        if (output_files.count("html"))
            html_file.open(path + "/html" + suffix);
        if (output_files.count("file"))
            file_file.open(path + "/file" + suffix);
        if (output_files.count("date"))
            date_file.open(path + "/date" + suffix);
    }

    void LangWriter::write(Record const &record, std::string const &chunk) {
        if (metadata_file.is_open())
            metadata_file.writeLine(toJSON(record, chunk, true).dump(-1, ' ', false, encoding_error));
        if (url_file.is_open())
            url_file.writeLine(record.getURL());
        if (mime_file.is_open())
            mime_file.writeLine(record.getHTTPcontentType());
        if (file_file.is_open())
            file_file.writeLine(record.getFilename() + ":" + std::to_string(record.getOffset()) + ":" + std::to_string(record.getSize()));
        if (date_file.is_open())
            date_file.writeLine(record.getWARCdate());
        if (html_file.is_open()) {
            if (format == Format::json)
                html_file.writeLine(toJSON(record.getPayload(), "h", encoding_error));
            else
                html_file.writeLine(util::encodeBase64(record.getPayload()));
        }
        if (text_file.is_open()) {
            if (format == Format::json)
                text_file.writeLine(toJSON(chunk, "p", encoding_error));
            else
                text_file.writeLine(util::encodeBase64(chunk));
        }
    }

    std::string get_paragraph_id(const std::string& text) {
        std::string result = "";
        std::vector<std::string> lines = util::split(text, "\n");

        while (!lines.empty() && lines[lines.size() - 1] == "") {
            lines.pop_back();
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            result += lines[i] + "\t" + std::to_string(i + 1) + ":" + std::to_string(lines.size()) + "\n";
        }

        return result;
    }

    void BilangWriter::write(const Record& record, [[maybe_unused]] bool skipped_extraction, bool paragraph_identification) {
        for (const auto& it : record.getTextByLangs()) {
            std::string chunk = it.second;

            if (paragraph_identification)
                chunk = get_paragraph_id(chunk);

            auto writer_it = writers.try_emplace(it.first, folder + "/" + it.first, output_files, compression, level, format, encoding_error);
            writer_it.first->second.write(record, chunk);
        }
    }

    void JSONLinesWriter::write(const Record& record, bool skipped_extraction, [[maybe_unused]] bool paragraph_identification) {
        // JSON lines format (https://jsonlines.org)
        if(skipped_extraction) {
            auto obj = toJSON(record, "", true);
            obj["h"] = record.getPayload();
            out_ << obj.dump(-1, ' ', false, encoding_error) << "\n";
            return;
        }
        for (auto &&it : record.getTextByLangs()) {
            std::string chunk = it.second;
            std::string lang = it.first;

            auto obj = toJSON(record, chunk, false);

            // Insert language if langid wasn't skipped
            if(lang != "")
                obj["l"] = lang;

            out_ << obj.dump(-1, ' ', false, encoding_error) << "\n";
        }
    }

    std::istream& operator>>(std::istream& in, Compression &c) {
        std::string token;
        in >> token;
        boost::algorithm::to_lower(token);
        namespace po = boost::program_options;

        if (token == "zstd") {
            c = Compression::zstd;
        } else if ("gzip"){
            c = Compression::gzip;
        } else {
            throw po::validation_error(po::validation_error::invalid_option_value);
        }

        return in;
    }
}

