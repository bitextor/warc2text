#include "warcreader.hh"
#include <boost/log/trivial.hpp>
#include <stdlib.h>

namespace warc2text {
    WARCReader::WARCReader(){
        warc_filename = "";
        file = nullptr;

        buf = new uint8_t[BUFFER_SIZE];
        scratch = new uint8_t[BUFFER_SIZE];

        s.zalloc = nullptr;
        s.zfree = nullptr;
        s.opaque = nullptr;
        s.avail_in = 0;
        s.next_in = buf;

        if (inflateInit2(&s, 32) != Z_OK) {
          BOOST_LOG_TRIVIAL(error) << "Failed to init zlib";
          abort();
        }
    }

    WARCReader::WARCReader(const std::string& filename) : WARCReader() {
        openFile(filename);
    }

    WARCReader::~WARCReader(){
        delete[] buf;
        delete[] scratch;
        inflateEnd(&s);
        closeFile();
    }

    bool WARCReader::getRecord(std::string& out, std::size_t max_size){
        int inflate_ret = 0;
        out.clear();
        std::size_t len;
        bool skip_record = false;
        while (inflate_ret != Z_STREAM_END) {
            if (s.avail_in == 0) {
                len = readChunk();
                if (len <= 0) {
                    // nothing more to read
                    out.clear();
                    return false;
                }
                s.avail_in = len;
                s.next_in = buf;
            }
            // inflate until either stream end is reached, or there is no more data
            while (inflate_ret != Z_STREAM_END && s.avail_in != 0) {
                s.next_out = scratch;
                s.avail_out = BUFFER_SIZE;
                inflate_ret = inflate(&s, Z_NO_FLUSH);
                if (inflate_ret != Z_OK && inflate_ret != Z_STREAM_END) {
                    BOOST_LOG_TRIVIAL(error) << "WARC " << warc_filename << ": error during decompressing";
                    out.clear();
                    return false;
                }
                if (not skip_record) out.append(scratch, scratch + (BUFFER_SIZE - s.avail_out));
                if (out.size() > max_size) {
                    BOOST_LOG_TRIVIAL(trace) << "WARC " << warc_filename << ": skipping large record";
                    out.clear();
                    skip_record = true;
                }
            }
            if (inflate_ret == Z_STREAM_END) {
                if (inflateReset(&s) != Z_OK) {
                  BOOST_LOG_TRIVIAL(error) << "Failed to reset zlib";
                  abort();
                }
                // next in and avail_in are updated while inflating, so no need to update them manually
            }
        }
        return true;
    }

    void WARCReader::openFile(const std::string& filename){
        warc_filename = filename;
        if (filename.empty() || filename == "-")
            file = std::freopen(nullptr, "rb", stdin); // make sure stdin is open in binary mode
        else file = std::fopen(filename.c_str(), "r");
        if (!file) {
            BOOST_LOG_TRIVIAL(error) << "WARC " << filename << ": file opening failed, skipping this WARC";
        }
    }

    void WARCReader::closeFile() {
        if (file) std::fclose(file);
    }

    std::size_t WARCReader::readChunk(){
        std::size_t len = std::fread(buf, sizeof(uint8_t), BUFFER_SIZE, file);
        if (std::ferror(file) && !std::feof(file)) {
            BOOST_LOG_TRIVIAL(error) << "WARC " << warc_filename << ": error during reading";
            return 0;
        }
        return len;
    }

    std::size_t WARCReader::tell() const {
        return std::ftell(file) - s.avail_in;
    }

} // warc2text
