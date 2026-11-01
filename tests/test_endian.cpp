#include <span>
#include <vector>
#include <array>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <itch/endian.hpp>

TEST_CASE("known bytes 16 bits") {
  const std::array<std::byte, 2> buf { std::byte(0x12), std::byte(0x34) };

  uint16_t read = itch::read_be16(std::span{buf});

  REQUIRE(read == 0x1234);
}

TEST_CASE("known bytes 32 bits") {
  const std::array<std::byte, 4> buf { 
    std::byte(0x12), 
    std::byte(0x34),
    std::byte(0x56),
    std::byte(0x78)
  };

  uint32_t read = itch::read_be32(std::span{buf});

  REQUIRE(read == 0x12345678);
}

TEST_CASE("known bytes 48 bits") {
  const std::array<std::byte, 6> buf { 
    std::byte(0x11), 
    std::byte(0x22),
    std::byte(0x33),
    std::byte(0x44),
    std::byte(0x55),
    std::byte(0x66)
  };

  uint64_t read = itch::read_be48(std::span{buf});

  REQUIRE(read == 0x112233445566);
}

TEST_CASE("known bytes 64 bits") {
  const std::array<std::byte, 8> buf { 
    std::byte(0x11), 
    std::byte(0x22),
    std::byte(0x33),
    std::byte(0x44),
    std::byte(0x55),
    std::byte(0x66),
    std::byte(0x77),
    std::byte(0x88)
  };

  uint64_t read = itch::read_be64(std::span{buf});

  REQUIRE(read == 0x1122334455667788);
}

TEST_CASE("known bytes 16 bits (high-bit bytes)") {
  const std::array<std::byte, 2> buf { std::byte(0xFF), std::byte(0xFE) };

  uint16_t read = itch::read_be16(std::span{buf});

  REQUIRE(read == 0xFFFE);
}

TEST_CASE("known bytes 32 bits (high-bit bytes)") {
  const std::array<std::byte, 4> buf { 
    std::byte(0xFF), 
    std::byte(0xFE),
    std::byte(0xFD),
    std::byte(0xFC)
  };

  uint32_t read = itch::read_be32(std::span{buf});

  REQUIRE(read == 0xFFFEFDFC);
}

TEST_CASE("known bytes 48 bits (high-bit bytes)") {
  const std::array<std::byte, 6> buf { 
    std::byte(0xFF), 
    std::byte(0xFE),
    std::byte(0xFD),
    std::byte(0xFC),
    std::byte(0xFB),
    std::byte(0xFA)
  };

  uint64_t read = itch::read_be48(std::span{buf});

  REQUIRE(read == 0xFFFEFDFCFBFA);
}

TEST_CASE("known bytes 64 bits (high-bit bytes)") {
  const std::array<std::byte, 8> buf { 
    std::byte(0xFF), 
    std::byte(0xFE),
    std::byte(0xFD),
    std::byte(0xFC),
    std::byte(0xFB),
    std::byte(0xFA),
    std::byte(0xF9),
    std::byte(0xF8)
  };

  uint64_t read = itch::read_be64(std::span{buf});

  REQUIRE(read == 0xFFFEFDFCFBFAF9F8);
}

TEST_CASE("read helpers return fixed-width unsigned types") {

  STATIC_REQUIRE(
      std::is_same_v<
        decltype(itch::read_be16(std::declval<std::span<const std::byte, 2>>())), 
        uint16_t
      >
  );

  STATIC_REQUIRE(
      std::is_same_v<
        decltype(itch::read_be32(std::declval<std::span<const std::byte, 4>>())), 
        uint32_t
      >
  );

  STATIC_REQUIRE(
      std::is_same_v<
        decltype(itch::read_be48(std::declval<std::span<const std::byte, 6>>())), 
        uint64_t
      >
  );

  STATIC_REQUIRE(
      std::is_same_v<
        decltype(itch::read_be64(std::declval<std::span<const std::byte, 8>>())), 
        uint64_t
      >
  );
}

TEST_CASE("unaligned offset") {
  const std::vector<std::byte> buf {
    std::byte(0x32),
    std::byte(0x49),
    std::byte(0x32), // start
    std::byte(0x51),
    std::byte(0x6F),
    std::byte(0xF9),
    std::byte(0xFF),
    std::byte(0xFE),
    std::byte(0x87),
    std::byte(0xA5), // end
    std::byte(0x00)
  };

  uint64_t read = itch::read_be64(std::span{buf}.subspan<2, 8>());

  REQUIRE(read == 0x32516FF9FFFE87A5);
}
