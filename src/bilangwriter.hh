#ifndef WARC2TEXT_WRITER_HH
#define WARC2TEXT_WRITER_HH

#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include "record.hh"
#include "zlib.h"

namespace warc2text {

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
    class GzipWriter {
        private:
            FILE* dest;
            z_stream s{};
            unsigned char* buf;
            void compress(const char* in, std::size_t size, int flush);

        public:
            GzipWriter();
            ~GzipWriter();
            void open(const std::string& filename);
            void close();
            void write(const char* text, std::size_t size);
            void writeLine(const char* text, std::size_t size);
            void writeLine(const std::string& text);
            bool is_open();
            static const std::size_t BUFFER_SIZE = 4096;
    };

    /**
     * Writes records to a specific folder for a specific language.
     */
    class LangWriter {
        private:
            GzipWriter metadata_file;
            GzipWriter url_file;
            GzipWriter mime_file;
            GzipWriter text_file;
            GzipWriter html_file;
            GzipWriter file_file;
            GzipWriter date_file;
        public:
            LangWriter(const std::string& folder, const std::unordered_set<std::string>& output_files);
            void write(const Record& record, const std::string &chunk);
    };

    class BilangWriter : public RecordWriter {
        private:
            std::string folder;
            std::unordered_set<std::string> output_files;
            std::unordered_map<std::string, LangWriter> writers;
        public:
            BilangWriter(const std::string& folder, const std::unordered_set<std::string>& output_files = {})
            : folder(folder)
            , output_files(output_files)
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
