#include "bilangwriter.hh"
#include "util.hh"

namespace warc2text{
    bool createDirectories(const std::string& path){
        if (!boost::filesystem::exists(path))
            return boost::filesystem::create_directories(path);
        else return false; // throw exception??
    }

    GzipWriter::GzipWriter() : file(), outbuf(), outstream(&outbuf) {
    }

    GzipWriter::~GzipWriter() {
        boost::iostreams::close(outbuf);
        file.close();
    }

    void GzipWriter::open(const std::string& filename) {
        file = std::ofstream(filename.data(), std::ios_base::out | std::ios_base::binary);
        outbuf.push(boost::iostreams::gzip_compressor()); // can specify compression level here with boost::iostreams::gzip_params
        outbuf.push(file);
        // outstream(&outbuf);
    }

    void GzipWriter::write(const char* text, unsigned int size) {
        outstream.write(text, size);
        outstream.write("\n", 1);
    }

    bool GzipWriter::is_open(){
        return file.is_open();
    }

    void BilangWriter::write(const Record& record) {
        const std::string* lang = &record.getLanguage();
        GzipWriter* url = &url_files[*lang];
        GzipWriter* mime = &mime_files[*lang];
        GzipWriter* text = &text_files[*lang];
        GzipWriter* html = &html_files[*lang];
        if (!url->is_open()) {
            // if one file does not exist, the rest shouldn't either
            std::string path = folder + "/" + *lang;
            createDirectories(path);
            url->open(path + "/url.gz");
            mime->open(path + "/mime.gz");
            text->open(path + "/text.gz");
            html->open(path + "/html.gz");
        }

        url->write(record.getURL().data(), record.getURL().size());
        // TODO: write actual content type
        mime->write(record.getHTTPcontentType().data(), record.getHTTPcontentType().size());
        std::string base64text;
        util::encodeBase64(record.getPlainText(), base64text);
        text->write(base64text.data(), base64text.size());
    }
}

