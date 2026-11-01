#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace itch {

inline uint16_t read_be16(std::span<const std::byte, 2> buf) {
  return static_cast<uint16_t>(
      std::to_integer<uint16_t>(buf[0]) << 8 
    | std::to_integer<uint16_t>(buf[1])
  );
}

inline uint32_t read_be32(std::span<const std::byte, 4> buf) {
  return static_cast<uint32_t>(
      std::to_integer<uint32_t>(buf[0]) << 24 
    | std::to_integer<uint32_t>(buf[1]) << 16 
    | std::to_integer<uint32_t>(buf[2]) << 8 
    | std::to_integer<uint32_t>(buf[3])
  );
}

inline uint64_t read_be48(std::span<const std::byte, 6> buf) {
  return static_cast<uint64_t>(
      std::to_integer<uint64_t>(buf[0]) << 40
    | std::to_integer<uint64_t>(buf[1]) << 32
    | std::to_integer<uint64_t>(buf[2]) << 24 
    | std::to_integer<uint64_t>(buf[3]) << 16
    | std::to_integer<uint64_t>(buf[4]) << 8 
    | std::to_integer<uint64_t>(buf[5])
  );
}

inline uint64_t read_be64(std::span<const std::byte, 8> buf) {
  return static_cast<uint64_t>(
      std::to_integer<uint64_t>(buf[0]) << 56 
    | std::to_integer<uint64_t>(buf[1]) << 48
    | std::to_integer<uint64_t>(buf[2]) << 40 
    | std::to_integer<uint64_t>(buf[3]) << 32
    | std::to_integer<uint64_t>(buf[4]) << 24 
    | std::to_integer<uint64_t>(buf[5]) << 16
    | std::to_integer<uint64_t>(buf[6]) << 8 
    | std::to_integer<uint64_t>(buf[7])
  );
}

} // namespace itch
