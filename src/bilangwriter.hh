#ifndef WARC2TEXT_WRITER_HH
#define WARC2TEXT_WRITER_HH

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include "record.hh"
#include "zlib.h"
#include "boost/iostreams/filtering_streambuf.hpp"

namespace warc2text {

    namespace bio = boost::iostreams;

    enum class Compression { zstd, gzip };

    /**
     * Generic interface for writing records to some form of output.
     */
    class RecordWriter {
    public:
        virtual void write(const Record& record, bool paragraph_identification = false) = 0;
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
        public:
            LangWriter(const std::string& folder, const std::unordered_set<std::string>& output_files,
                       Compression c = Compression::gzip, int l = 3);
            void write(const Record& record, const std::string &chunk);
    };

    class BilangWriter : public RecordWriter {
        private:
            std::string folder;
            std::unordered_set<std::string> output_files;
            std::unordered_map<std::string, LangWriter> writers;
            Compression compression;
            int level;
        public:
            BilangWriter(const std::string& folder, const std::unordered_set<std::string>& output_files = {},
                         Compression c = Compression::gzip, int l = 3)
            : folder(folder) , output_files(output_files) , compression(c) , level(l)
            {
                //
            };

            virtual void write(const Record& record, bool paragraph_identification = false);
    };

    class JSONLinesWriter : public RecordWriter {
        private:
            std::ostream &out_;
        public:
            explicit JSONLinesWriter(std::ostream &out) : out_(out) {};

            virtual void write(const Record& record, bool paragraph_identification = false);
    };
}

#endif
