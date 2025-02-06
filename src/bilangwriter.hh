#ifndef WARC2TEXT_WRITER_HH
#define WARC2TEXT_WRITER_HH

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include "record.hh"
#include "zlib.h"
#include <nlohmann/json.hpp>
#include "boost/iostreams/filtering_streambuf.hpp"

namespace warc2text {

    namespace bio = boost::iostreams;

    using json_error = nlohmann::ordered_json::error_handler_t;

    enum class Compression { zstd, gzip };

    enum class Format { b64, json };

    /**
     * Generic interface for writing records to some form of output.
     */
    class RecordWriter {
    public:
        virtual void write(const Record& record, bool skipped_extraction, bool paragraph_identification = false) = 0;
        virtual ~RecordWriter() = default;
    };

    /**
     * Writer used by BilangWriter to write a single compressed file
     * (i.e. a column for a specific language)
     */
    class CompressWriter {
        private:
            std::ofstream file;
            bio::filtering_streambuf<bio::output> compressor;
            Compression compression;
            int level;

        public:
            CompressWriter();
            CompressWriter(Compression c, int l);
            ~CompressWriter();
            void open(const std::string& filename);
            void close();
            void writeLine(const std::string& text);
            bool is_open();
    };

    /**
     * Writes records to a specific folder for a specific language.
     */
    class LangWriter {
        private:
            CompressWriter metadata_file;
            CompressWriter url_file;
            CompressWriter mime_file;
            CompressWriter text_file;
            CompressWriter html_file;
            CompressWriter file_file;
            CompressWriter date_file;
            Format format;
            json_error encoding_error;

            std::string format_text(const std::string &text, std::string field_name);
        public:
            LangWriter(const std::string& folder, const std::unordered_set<std::string>& output_files,
                       Compression c = Compression::gzip, int l = 3, Format f = Format::b64,
                       json_error e = json_error::replace);
            void write(const Record& record, const std::string &chunk);
    };

    class BilangWriter : public RecordWriter {
        private:
            std::string folder;
            std::unordered_set<std::string> output_files;
            std::unordered_map<std::string, LangWriter> writers;
            Compression compression;
            int level;
            Format format;
            json_error encoding_error;
        public:
            BilangWriter(const std::string& folder, const std::unordered_set<std::string>& output_files = {},
                         Compression c = Compression::gzip, int l = 3, Format f = Format::b64,
                         json_error e = json_error::replace)
            : folder(folder) , output_files(output_files) , compression(c) , level(l), format(f), encoding_error(e)
            {
                //
            };

            virtual void write(const Record& record, bool skipped_extraction, bool paragraph_identification = false);
    };

    class JSONLinesWriter : public RecordWriter {
        private:
            std::ostream &out_;
            json_error encoding_error;
        public:
            explicit JSONLinesWriter(std::ostream &out, json_error e) : out_(out), encoding_error(e) {};

            virtual void write(const Record& record, bool skipped_extraction, bool paragraph_identification = false);
    };
}

#endif
