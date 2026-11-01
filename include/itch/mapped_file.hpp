#pragma once

#include <string>
#include <utility>
#include <system_error>
#include <cerrno>
#include <span>
#include <format>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace itch {

class MappedFile {

private:
  const std::byte* buf_ = nullptr;
  size_t size_ = 0;

  void release() noexcept {
    if (buf_ != nullptr) {
      int status = munmap(const_cast<void*>(reinterpret_cast<const void*>(buf_)), size_);
      assert(status == 0);
    }
  }

public:
  MappedFile(const MappedFile& other) = delete;
  MappedFile& operator=(const MappedFile& other) = delete;

  MappedFile(MappedFile&& other) noexcept : buf_(other.buf_), size_(other.size_) {
    other.buf_ = nullptr;
    other.size_ = 0;
  }
  MappedFile& operator=(MappedFile&& other) noexcept {
    if (this != &other) {
      release();

      buf_ = std::exchange(other.buf_, nullptr);
      size_ = std::exchange(other.size_, 0);
    }

    return *this;
  }

  explicit MappedFile(const std::string& path) {

    struct FdGuard {

      FdGuard() = default;
      FdGuard(const FdGuard& other) = delete;
      FdGuard& operator=(const FdGuard& other) = delete;

      int fd = -1;

      ~FdGuard() {
        if (fd >= 0) {
          int status = close(fd);
          assert(status == 0);
        }
      }
    };

    FdGuard fdg;

    fdg.fd = open(path.c_str(), O_RDONLY);

    if (fdg.fd == -1) {
      int err = errno;
      throw std::system_error(err, std::generic_category(), "open " + path);
    }

    struct stat st;

    if (fstat(fdg.fd, &st) == -1)  {
      int err = errno;
      throw std::system_error(err, std::generic_category(), "fstat " + path);
    }

    size_ = static_cast<size_t>(st.st_size);

    if (size_ == 0) {
      return;
    }

    void* buf = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fdg.fd, 0);

    if (buf == MAP_FAILED) {
      int err = errno;
      throw std::system_error(
        err,
        std::generic_category(),
        std::format("mmap size {} fd {}", size_, fdg.fd)
      );
    }

    buf_ = reinterpret_cast<const std::byte*>(buf);
  }

  std::span<const std::byte> bytes() const {
    return {buf_, size_};
  }

  ~MappedFile() {
    release();
  }
};

} // namespace itch
