#include "bilangwriter.hh"
#include "util.hh"
#include "util/exception.hh"
#include <cassert>
#include <string>
#include <iomanip>
#include <boost/json.hpp>


namespace warc2text{

    GzipWriter::GzipWriter()
    : dest(nullptr),
      buf(new unsigned char[BUFFER_SIZE]) {
        //
    }

    GzipWriter::~GzipWriter() {
        if (is_open())
            close();
        delete[] buf;
    }

    void GzipWriter::compress(const char *in, std::size_t size, int flush) {
        assert(is_open());
        if (size == 0 && flush == Z_NO_FLUSH) return;
        s.avail_in = size;
        s.next_in = (Bytef *) in;
        s.avail_out = 0;
        s.next_out = buf;
        int ret = Z_OK;
        //std::size_t written;
        while (s.avail_out == 0) {
            s.avail_out = BUFFER_SIZE;
            s.next_out = buf;
            ret = deflate(&s, flush);
            assert(ret == Z_OK || ret == Z_STREAM_END); // Z_STREAM_END only happens if flush == Z_FINISH
            std::size_t compressed = BUFFER_SIZE - s.avail_out;
            //written = std::fwrite(buf, 1, compressed, dest);
            std::fwrite(buf, 1, compressed, dest);
            // TODO error handling
            // if (written != compressed || std::ferror(dest)) {
            // }
        }
        assert(s.avail_in == 0);
    }

    void GzipWriter::open(const std::string& filename) {
        dest = std::fopen(filename.c_str(), "wb");
        UTIL_THROW_IF(!dest, util::ErrnoException, "while creating " << filename);
        s.zalloc = nullptr;
        s.zfree = nullptr;
        s.opaque = nullptr;
        int ret = deflateInit2(&s, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
        assert(ret == Z_OK);
    }

    void GzipWriter::close() {
        compress("", 0, Z_FINISH);
        deflateEnd(&s);
        std::fclose(dest);
        dest = nullptr;
    }

    void GzipWriter::write(const char* text, std::size_t size) {
        compress(text, size, Z_NO_FLUSH);
    }

    void GzipWriter::writeLine(const char* text, std::size_t size) {
        compress(text, size, Z_NO_FLUSH);
        compress("\n", 1, Z_NO_FLUSH);
    }

    void GzipWriter::writeLine(const std::string& text) {
        compress(text.c_str(), text.size(), Z_NO_FLUSH);
        compress("\n", 1, Z_NO_FLUSH);
    }

    bool GzipWriter::is_open(){
        return dest != nullptr;
    }

    LangWriter::LangWriter(const std::string& path, const std::unordered_set<std::string>& output_files) {
        util::createDirectories(path);

        if (output_files.count("url"))
            url_file.open(path + "/url.gz");
        if (output_files.count("text"))
            text_file.open(path + "/text.gz");
        if (output_files.count("mime"))
            mime_file.open(path + "/mime.gz");
        if (output_files.count("html"))
            html_file.open(path + "/html.gz");
        if (output_files.count("file"))
            file_file.open(path + "/file.gz");
    }

    void LangWriter::write(Record const &record, std::string const &chunk) {
        if (url_file.is_open())
            url_file.writeLine(record.getURL());
        if (mime_file.is_open())
            mime_file.writeLine(record.getHTTPcontentType());
        if (file_file.is_open())
            file_file.writeLine(record.getFilename() + ":" + std::to_string(record.getOffset()) + ":" + std::to_string(record.getSize()));
        if (html_file.is_open())
            html_file.writeLine(util::encodeBase64(record.getPayload()));
        if (text_file.is_open())
            text_file.writeLine(util::encodeBase64(chunk));
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

    void BilangWriter::write(const Record& record, bool paragraph_identification) {
        for (const auto& it : record.getTextByLangs()) {
            std::string chunk = it.second;

            if (paragraph_identification)
                chunk = get_paragraph_id(chunk);

            auto writer_it = writers.try_emplace(it.first, folder + "/" + it.first, output_files);
            writer_it.first->second.write(record, chunk);
        }
    }

    void JSONLinesWriter::write(const Record& record, [[maybe_unused]] bool paragraph_identification) {
        // JSON lines format (https://jsonlines.org)
        for (auto &&chunk : record.getTextByLangs()) {
            out_ << boost::json::value{
                 {"f", boost::json::string(record.getFilename())},
                 {"o", boost::json::value(record.getOffset())},
                 {"s", boost::json::value(record.getSize())},
                 {"rs", boost::json::value(record.getPayload().size())},
                 {"ps", boost::json::value(chunk.second.size())},
                 {"l", boost::json::string(chunk.first)},
                 {"u", boost::json::string(record.getURL())},
                 {"c", boost::json::string(record.getHTTPcontentType())},
                 {"p", boost::json::string(chunk.second)},
            } << "\n";
        }
    }
}

