#include "bilangwriter.hh"

namespace warc2text{
    bool createDirectory(const std::string& path){
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
        if (!url->is_open()) {
            // if one file does not exist, the rest shouldn't either
            std::string path = folder + "/" + *lang;
            createDirectory(path);
            url->open(path + "/url.gz");
            mime->open(path + "/mime.gz");
            text->open(path + "/text.gz");
        }

        url->write(record.getHeaderProperty("WARC-Target-URI").data(), record.getHeaderProperty("WARC-Target-URI").size());
        std::string type = "text/html";
        mime->write(type.data(), type.size());
        text->write(record.getPlainText().data(), record.getPlainText().size());
    }
}

