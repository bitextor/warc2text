#include "zipreader.hh"
#include <boost/crc.hpp>

namespace util {

ZipReader::ZipReader(const std::string &payload)
: src_(nullptr, &zip_source_free), archive_() {
    zip_error_t error{};

    src_.reset(zip_source_buffer_create(const_cast<void*>(static_cast<const void*>(payload.data())), payload.size(), 0, &error));
    if (!src_)
        throw ZipReadError(&error);

    zip_source_keep(src_.get());

    archive_.reset(zip_open_from_source(src_.get(), 0, &error), zip_discard);
    if (!archive_)
        throw ZipReadError(&error);
}

size_t ZipReader::size() const {
    zip_int64_t num_entries = zip_get_num_entries(archive_.get(), 0);
    return num_entries;
}

ZipEntry::ZipEntry(std::shared_ptr<zip_t> archive, size_t index)
: archive_(archive), index_(index) {
    //
}

size_t ZipEntry::index() const {
    return index_;
}

std::string ZipEntry::name() const {
    const char *name = zip_get_name(archive_.get(), index_, 0);
    return std::string(name);
}

size_t ZipEntry::size() const {
    zip_stat_t st;
    zip_stat_init(&st);
    zip_stat_index(archive_.get(), index_, 0, &st);
    return st.size;
}

std::string ZipEntry::read(std::string &&buffer) const {
    // Remove any lingering error state from previous reads
    zip_error_clear(archive_.get());

    // Get stats for size and checksum
    zip_stat_t st;
    zip_stat_init(&st);
    zip_stat_index(archive_.get(), index_, 0, &st);
    
    // Open pointer to file inside zip
    std::unique_ptr<zip_file_t, decltype(&zip_fclose)> fh(zip_fopen_index(archive_.get(), index_, 0), &zip_fclose);
    if (!fh)
        throw ZipReadError(zip_get_error(archive_.get()));

    // Make room for uncompressed data
    buffer.resize(st.size);
    
    // Read uncompressed data (in a loop in case it can't be done in one read?)
    for (size_t read = 0; read < buffer.size();) {
        auto len = zip_fread(fh.get(), &buffer[0] + read, buffer.size() - read);

        if (len == -1)
            throw ZipReadError(zip_get_error(archive_.get()));

        read += len;
    }

    // Loop condition already makes sure we read exactly buffer.size() bytes
    // buffer.resize(read);

    // Check CRC checksum
    boost::crc_32_type crc;
    crc.process_bytes(buffer.data(), buffer.size());
    if (crc.checksum() != st.crc)
        throw ZipReadError("bad CRC");

    return std::move(buffer);
}

std::string ZipEntry::read() const {
    return read(std::string());
}

} // end namespace
