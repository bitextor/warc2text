#include <string>
#include <memory>
#include <zip.h>

namespace util {

class ZipReadError : public std::runtime_error {
public:
    ZipReadError(std::string const &error) : runtime_error(error) {
        //
    }

    ZipReadError(zip_error_t *error) : runtime_error(zip_error_strerror(error)) {
        //
    }
};

class ZipEntry {
private:
    std::shared_ptr<zip_t> archive_;
    size_t index_;
public:
    ZipEntry(std::shared_ptr<zip_t> archive, size_t index);

    size_t index() const;
    std::string name() const;
    size_t size() const;
    std::string read(std::string &&buffer) const;
    std::string read() const;

    friend bool operator==(const ZipEntry& a, const ZipEntry& b) {
        return a.archive_ == b.archive_ && a.index_ == b.index_;
    };

    friend bool operator!=(const ZipEntry& a, const ZipEntry& b) {
        return a.archive_ != b.archive_ || a.index_ != b.index_;
    };

    friend class ZipEntryIterator;
};

class ZipEntryIterator {
private:
    ZipEntry entry_;
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = int;
    using value_type        = ZipEntry;
    using pointer           = ZipEntry const *;
    using reference         = ZipEntry const &;

    ZipEntryIterator(std::shared_ptr<zip_t> archive, size_t index)\
    : entry_(archive, index) {
        //
    }

    reference operator*() const {
        return entry_;
    }

    pointer operator->() const {
        return &entry_;
    }

    // Prefix increment
    ZipEntryIterator& operator++() {
        entry_.index_++;
        return *this;
    }

    // Postfix increment
    ZipEntryIterator operator++(int) {
        ZipEntryIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const ZipEntryIterator& a, const ZipEntryIterator& b) {
        return a.entry_ == b.entry_;
    };

    friend bool operator!=(const ZipEntryIterator& a, const ZipEntryIterator& b) {
        return a.entry_ != b.entry_;
    };
};

/**
 * Class to iterate over files in an in-memory zip archive.
 *
 * Example:
 *
 *   ZipReader reader(buffer);
 *   for (auto file : reader)
 *     if (file.name() == "something.txt")
 *       std::cerr << file.read();
 *
 */
class ZipReader {
private:
    std::unique_ptr<zip_source_t, decltype(&zip_source_free)> src_;
    std::shared_ptr<zip_t> archive_;

public:
    typedef ZipEntryIterator const_iterator;

    ZipReader(const std::string &payload);

    size_t size() const;

    const_iterator begin() const {
        return ZipEntryIterator(archive_, 0);
    }

    const_iterator end() const {
        return ZipEntryIterator(archive_, size());
    }
};

} // end namespace
