#include "bilangwriter.hh"
#include "util.hh"
#include "util/exception.hh"
#include <cassert>
#include <string>
#include <iomanip>


namespace {
    /**
     * Little bit of JSON wrapping to make sure we only ever print safe values
     */

    template <typename T>
    struct JSONValue {
        const T& ref;
    };

    template <typename T>
    JSONValue<T> escapeJSON(T const &ref) {
        return JSONValue<T>{ref};
    }

    std::ostream &operator<<(std::ostream &out, JSONValue<std::size_t> const &val) {
        return out << val.ref;
    }

    std::ostream &operator<<(std::ostream &out, JSONValue<std::string> const &val) {
        return out << std::quoted(val.ref);
    }
}

namespace warc2text{

    GzipWriter::GzipWriter() {
        dest = nullptr;
        compressed = 0;
        s.zalloc = nullptr;
        s.zfree = nullptr;
        s.opaque = nullptr;
        int ret = deflateInit2(&s, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
        assert(ret == Z_OK);
        buf = new unsigned char[BUFFER_SIZE];
    }

    GzipWriter::~GzipWriter() {
        if (dest) {
            this->compress("", 0, Z_FINISH);
            deflateEnd(&s);
            std::fclose(dest);
        }
        delete[] buf;
    }

    void GzipWriter::compress(const char *in, std::size_t size, int flush) {
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
            compressed = BUFFER_SIZE - s.avail_out;
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
    }

    void GzipWriter::write(const char* text, std::size_t size) {
        this->compress(text, size, Z_NO_FLUSH);
    }

    void GzipWriter::writeLine(const char* text, std::size_t size) {
        this->compress(text, size, Z_NO_FLUSH);
        this->compress("\n", 1, Z_NO_FLUSH);
    }

    void GzipWriter::writeLine(const std::string& text) {
        this->compress(text.c_str(), text.size(), Z_NO_FLUSH);
        this->compress("\n", 1, Z_NO_FLUSH);
    }

    bool GzipWriter::is_open(){
        return dest != nullptr;
    }

    void BilangWriter::write(const std::string& lang, const std::string& b64text, const std::string& url, const std::string& mime, const std::string& b64html, const std::string& file) {
        GzipWriter* gzurl = &url_files[lang];
        GzipWriter* gztext = &text_files[lang];
        GzipWriter* gzmime = nullptr;
        GzipWriter* gzhtml = nullptr;
        GzipWriter* gzfile = nullptr;
        if (output_files.count("mime") == 1) gzmime = &(mime_files[lang]);
        if (output_files.count("html") == 1) gzhtml = &(html_files[lang]);
        if (output_files.count("file") == 1) gzfile = &(file_files[lang]);
        if (!gzurl->is_open()) {
            // if one file does not exist, the rest shouldn't either
            std::string path = folder + "/" + lang;
            util::createDirectories(path);
            gzurl->open(path + "/url.gz");
            gztext->open(path + "/text.gz");
            if (gzmime != nullptr) gzmime->open(path + "/mime.gz");
            if (gzhtml != nullptr) gzhtml->open(path + "/html.gz");
            if (gzfile != nullptr) gzfile->open(path + "/file.gz");
        }

        gzurl->writeLine(url);
        gztext->writeLine(b64text);
        if (gzmime != nullptr) gzmime->writeLine(mime);
        if (gzhtml != nullptr) gzhtml->writeLine(b64html);
        if (gzfile != nullptr) gzfile->writeLine(file);
    }

    std::string get_paragraph_id(const std::string& text) {
        std::string result = "";
        std::vector<std::string> lines = util::split(text, "\n");

        while (lines[lines.size() - 1] == "") {
            lines.pop_back();
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            result += lines[i] + "\t" + std::to_string(i + 1) + ":" + std::to_string(lines.size()) + "\n";
        }

        return result;
    }

    void BilangWriter::write(const Record& record, bool paragraph_identification) {
        std::string base64text;
        std::string base64html;

        if (output_files.count("html") == 1)
            util::encodeBase64(record.getPayload(), base64html);

        std::string file = record.getFilename() + ":" + std::to_string(record.getOffset()) + ":" + std::to_string(record.getSize());

        for (const auto& it : record.getTextByLangs()) {
            std::string payload = it.second;

            if (paragraph_identification) {
                payload = get_paragraph_id(payload);
            }

            util::encodeBase64(payload, base64text);
            this->write(it.first, base64text, record.getURL(), record.getHTTPcontentType(), base64html, file);
        }
    }

    void JSONLinesWriter::write(const Record& record, [[maybe_unused]] bool paragraph_identification) {
        // JSON lines format (https://jsonlines.org)
        out_ << "{"
             << "\"f\":"   << escapeJSON(record.getFilename()) << ","
             << "\"o\":"   << escapeJSON(record.getOffset()) << ","
             << "\"s\":"   << escapeJSON(record.getSize()) << ","
             << "\"rs\":"  << escapeJSON(record.getPayload().size()) << ","
             << "\"ps\":"  << escapeJSON(record.getPlainText().size()) << ","
             << "\"l\":" << escapeJSON(record.getLanguage()) << ","
             << "\"u\":" << escapeJSON(record.getURL()) << ","
             << "\"c\":" << escapeJSON(record.getHTTPcontentType())
             << "}\n";
    }
}

