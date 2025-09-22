#include <string>
#include <stdexcept>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <sstream>

namespace warc2text {
    const std::unordered_set<std::string> noncompressed_content_encodings = {"none", "identity", "raw", "utf-8"}; // a few most popular Content-Encoding types that do not require decompression


       
    void decompress_gzip_boost2(const std::string& compressed, std::string& res) {
        std::istringstream in_stream(compressed, std::ios_base::binary);
        std::ostringstream out_stream(std::ios_base::binary);

        boost::iostreams::filtering_istream in;
        try {
            in.push(boost::iostreams::gzip_decompressor());
            in.push(in_stream);

            // copy will throw if decompression fails
            boost::iostreams::copy(in, out_stream);
        } catch (const std::exception& e) {
            // leave 'compressed' intact and rethrow as runtime_error
            throw std::invalid_argument(std::string("gzip decompression failed: ") + e.what());
        }
        res = out_stream.str();
    }


    void decompress_gzip_boost1(std::string& compressed, std::string& res) {
        std::string decompressed;

        try {
            boost::iostreams::filtering_ostream os;
            os.push(boost::iostreams::gzip_decompressor());
            os.push(boost::iostreams::back_inserter(decompressed));

            // Write the compressed data to the stream
            os.write(compressed.data(), compressed.size());
            os.flush(); // Ensure all data is flushed to the output
        } catch (const std::exception& e) {
            throw std::invalid_argument("HTTP response decompression failed: " + std::string(e.what()));
        }

        res = decompressed;
    }


    void decompress_gzip(std::string& compressed) {
        if (compressed.size() < 2 || (static_cast<unsigned char>(compressed[0]) != 0x1F) || (static_cast<unsigned char>(compressed[1]) != 0x8B))
           throw std::invalid_argument("not a gzip-ed string");

        decompress_gzip_boost1(compressed, compressed);
    }
 
    void decompress_gzip_compare(std::string& compressed) {
        if (compressed.size() < 2 || (static_cast<unsigned char>(compressed[0]) != 0x1F) || (static_cast<unsigned char>(compressed[1]) != 0x8B))
           throw std::invalid_argument("not a gzip-ed string");
        std::string r1, r2;
        try {
            decompress_gzip_boost1(compressed, r1);
        } catch (const std::exception& e) {
            r1 = "exception";
            r1 += e.what();
        } catch (...) {
            r1 = "exception: not std::exception";
        }
        try {
            decompress_gzip_boost2(compressed, r2);
        } catch (const std::exception& e) {
            r2 = "exception: ";
            r2 += e.what();
        } catch (...) {
            r2 = "exception: not std::exception";
        }

        if (r1 != r2)
            BOOST_LOG_TRIVIAL(warning) << "different decompression results: " << r1.size() <<" and " << r2.size() << ":" << std::endl << r1 << std::endl << r2 <<std::endl; 
//        if (compressed.size() > decompressed.size())
//            BOOST_LOG_TRIVIAL(warning) << "gzip compression ratio < 1 (" << 1.0*decompressed.size()/compressed.size() << "x)," << decompressed.size() << " bytes after decompressing " << compressed.size() << " bytes:" << std::endl;
//BOOST_LOG_TRIVIAL(warning) << "gzip compression ratio " << 1.0*decompressed.size()/compressed.size() << "x," << decompressed.size() << " bytes after decompressing " << compressed.size() << " bytes:" << compressed.substr(0,20) << std::endl;

    }

    void decompress(std::string& payload, const std::string& encoding) {
        if (encoding == "gzip" || encoding == "x-gzip")
            decompress_gzip(payload);
        else if (noncompressed_content_encodings.count(encoding) > 0)
            ; // do nothing for content encodings that do not require decompression
        else if (encoding == "br" || encoding == "deflate")
            throw std::invalid_argument("Unsupported HTTP Content-Encoding:" + encoding);               
        else           
            throw std::invalid_argument("Non-standard HTTP Content-Encoding:" + encoding);               
    }

    void dechunk(std::string& input) {
        // Assuming grammar:
        // (<size s><space>*\r\n<chunk of size s>\r\n)+0(\r\n)?
        // where <size s> is a positive hexadecimal number showing the size of the following chunk
        size_t pos = 0;
        size_t chunk_size, processed;
        while (pos < input.length()) {
            size_t lineEnd = input.find("\r\n", pos);
            if (lineEnd == std::string::npos)
                lineEnd = input.length();
            std::string line = input.substr(pos, lineEnd - pos);
            chunk_size = std::stoul(line, &processed, 16); // throws std::invalid_argument and std::out_of_range
            if (processed - 1 < line.find_last_not_of(" "))  // should be true also for std::string::npos
                throw std::invalid_argument("chunk size line has unrecognized format: '" + input.substr(pos, lineEnd - pos) + "'");
            //std::cout << "|" << input.substr(pos, lineEnd - pos) << "|" << std::endl;
            input.erase(pos, lineEnd - pos + 2);  // remove the whole line with the chunk size => merge chunks
            if (chunk_size == 0) break;

            pos += chunk_size;
            if (pos >= input.length())
                throw std::invalid_argument("the specified chunk size is larger than the remaining part of the string");
            if (input.find("\r\n", pos) != pos)
                throw std::invalid_argument("cannot find CRLF immediately after a chunk: '" + input.substr(pos) + "'");
            input.erase(pos, 2); // remove \r\n after the current chunk before the next chunk size
        }
    }
 
}
