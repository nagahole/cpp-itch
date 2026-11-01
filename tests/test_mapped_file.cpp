#include <vector>
#include <span>
#include <cstddef>
#include <system_error>
#include <algorithm>
#include <utility>

#include <filesystem>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include <itch/mapped_file.hpp>


class TempFile {
private:
  std::string path_;

public:
  TempFile(const TempFile& other) = delete;
  TempFile& operator=(const TempFile& other) = delete;

  explicit TempFile(
      const std::filesystem::path& path, std::span<const std::byte> contents
  ) {
    path_ = path;
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    REQUIRE(file.is_open());

    file.write(
      reinterpret_cast<const char*>(contents.data()),
      static_cast<std::streamsize>(contents.size())
    );
    file.close();
  }

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path_, ec);
  }
};


TEST_CASE("known bytes") {
  const auto path = std::filesystem::temp_directory_path()
    / "mapped_file_test_known_bytes.bin";

  std::vector<std::byte> bytes{
    std::byte(0x12),
    std::byte(0x34),
    std::byte(0x56),
    std::byte(0x78),
    std::byte(0x90)
  };

  TempFile file(path, std::span<const std::byte>{bytes});

  const itch::MappedFile mapped_file(path);
  std::span<const std::byte> mapped_bytes = mapped_file.bytes();

  REQUIRE(std::ranges::equal(bytes, mapped_bytes));
}

TEST_CASE("nonexistent path") {
  const auto path = std::filesystem::temp_directory_path()
    / "mapped_file_nonexistent_path.bin";

  std::filesystem::remove(path);
  REQUIRE_THROWS_AS(itch::MappedFile(path), std::system_error);
}

TEST_CASE("empty file") {
  const auto path = std::filesystem::temp_directory_path()
    / "mapped_file_empty_file.bin";

  TempFile file(path, {});

  const itch::MappedFile mapped_file(path);
  std::span<const std::byte> mapped_bytes = mapped_file.bytes();

  REQUIRE(mapped_bytes.size() == 0);
}

TEST_CASE("move") {
  const auto path = std::filesystem::temp_directory_path()
    / "mapped_file_move.bin";

  std::vector<std::byte> bytes {
    std::byte(0x12),
    std::byte(0x34),
    std::byte(0x56),
    std::byte(0x78),
    std::byte(0x90)
  };

  TempFile file(path, std::span<const std::byte>{bytes});

  itch::MappedFile mapped_file(path);

  std::vector<itch::MappedFile> vec;
  vec.push_back(std::move(mapped_file));

  REQUIRE(mapped_file.bytes().empty());

  std::span<const std::byte> mapped_bytes = vec[0].bytes();

  REQUIRE(std::ranges::equal(bytes, mapped_bytes));
}

