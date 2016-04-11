#pragma once
#include <array>
#include <ostream>
#include <string>
#include <cstdint>

namespace ice {

struct uuid {
  std::uint32_t time_low = 0;
  std::uint16_t time_mid = 0;
  std::uint16_t time_hi_and_version = 0;
  std::uint8_t clk_seq_hi_res = 0;
  std::uint8_t clk_seq_low = 0;
  std::array<std::uint8_t, 6> node;

  uuid() = default;
  uuid(const std::string& buffer);

  std::string str() const;
  void str(const std::string& buffer);

  static uuid generate();
};

inline constexpr bool operator==(const ice::uuid& a, const ice::uuid& b)
{
  return
    a.time_low == b.time_low &&
    a.time_mid == b.time_mid &&
    a.time_hi_and_version == b.time_hi_and_version &&
    a.clk_seq_hi_res == b.clk_seq_hi_res &&
    a.clk_seq_low == b.clk_seq_low &&
    a.node == b.node;
}

inline constexpr bool  operator!=(const ice::uuid& a, const ice::uuid& b)
{
  return !operator==(a, b);
}

std::ostream& operator<<(std::ostream& os, const ice::uuid& uuid);

}  // namespace ice