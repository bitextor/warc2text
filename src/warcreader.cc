#include "warcreader.hh"
#include <cassert>

namespace warc2text {
    WARCReader::WARCReader(){
        file = nullptr;

        buf = new uint8_t[BUFFER_SIZE];
        scratch = new uint8_t[BUFFER_SIZE];

        s.zalloc = nullptr;
        s.zfree = nullptr;
        s.opaque = nullptr;
        s.avail_in = 0;
        s.next_in = buf;

        assert(inflateInit2(&s, 32) == Z_OK);
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

    bool WARCReader::getRecord(std::string& out){
        int inflate_ret = 0;
        out.clear();
        std::size_t len;
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
                assert(inflate_ret == Z_OK || inflate_ret == Z_STREAM_END);
                out.append(scratch, scratch + (BUFFER_SIZE - s.avail_out));
            }
            if (inflate_ret == Z_STREAM_END) {
                assert(inflateReset(&s) == Z_OK);
                // next in and avail_in are updated while inflating, so no need to update them manually
            }
        }
        return true;
    }

    void WARCReader::openFile(const std::string& filename){
        if (filename.empty() || filename == "-")
            file = std::freopen(nullptr, "rb", stdin); // make sure stdin is open in binary mode
        else file = std::fopen(filename.c_str(), "r");
        if (!file) {
            std::perror("File opening failed");
            exit(1);
        }
    }

    std::size_t WARCReader::readChunk(){
        std::size_t len = std::fread(buf, sizeof(uint8_t), BUFFER_SIZE, file);
        if (std::ferror(file) && !std::feof(file)) {
            std::perror("Error during reading");
            exit(1);
        }
        return len;
    }

} // warc2text
