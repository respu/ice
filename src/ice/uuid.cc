#include <ice/uuid.h>
#include <openssl/rand.h>
#include <algorithm>
#include <stdexcept>
#include <cstdio>

namespace ice {

uuid::uuid(const std::string& buffer)
{
  str(buffer);
}

std::string uuid::str() const
{
  std::string buffer;
  buffer.resize(40);
  auto size = std::snprintf(&buffer[0], buffer.size(), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    time_low, time_mid, time_hi_and_version, clk_seq_hi_res, clk_seq_low,
    node[0], node[1], node[2], node[3], node[4], node[5]);
  if (size < 0) {
    throw std::runtime_error("uuid: buffer error");
  }
  buffer.resize(static_cast<std::size_t>(size));
  return buffer;
}

void uuid::str(const std::string& buffer)
{
  ice::uuid uuid;
  unsigned int data[11];
  auto count = std::sscanf(buffer.c_str(), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    &data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6], &data[7], &data[8], &data[9], &data[10]);
  if (count != 11) {
    throw std::runtime_error("uuid: format error");
  }
  time_low = static_cast<std::uint32_t>(data[0]);
  time_mid = static_cast<std::uint16_t>(data[1]);
  time_hi_and_version = static_cast<std::uint16_t>(data[2]);
  clk_seq_hi_res = static_cast<std::uint8_t>(data[3]);
  clk_seq_low = static_cast<std::uint8_t>(data[4]);
  node[0] = static_cast<std::uint8_t>(data[5]);
  node[1] = static_cast<std::uint8_t>(data[6]);
  node[2] = static_cast<std::uint8_t>(data[7]);
  node[3] = static_cast<std::uint8_t>(data[8]);
  node[4] = static_cast<std::uint8_t>(data[9]);
  node[5] = static_cast<std::uint8_t>(data[10]);
}

uuid uuid::generate()
{
  union uuid_data {
    struct {
      std::uint32_t time_low;
      std::uint16_t time_mid;
      std::uint16_t time_hi_and_version;
      std::uint8_t clk_seq_hi_res;
      std::uint8_t clk_seq_low;
      std::uint8_t node[6];
    };
    uint8_t rnd[16];
    uuid_data() {}
  } data;
  
  if (!RAND_bytes(data.rnd, sizeof(data))) {
    throw std::runtime_error("uuid: random bytes error");
  }

  // RFC-4122 Section 4.2
  data.clk_seq_hi_res = static_cast<uint8_t>((data.clk_seq_hi_res & 0x3F) | 0x80);
  data.time_hi_and_version = static_cast<uint16_t>((data.time_hi_and_version & 0x0FFF) | 0x4000);

  ice::uuid uuid;
  uuid.time_low = data.time_low;
  uuid.time_mid = data.time_mid;
  uuid.time_hi_and_version = data.time_hi_and_version;
  uuid.clk_seq_hi_res = data.clk_seq_hi_res;
  uuid.clk_seq_low = data.clk_seq_low;
  std::copy(data.node, data.node + sizeof(data.node), uuid.node.begin());

  return uuid;
}

std::ostream& operator<<(std::ostream& os, const ice::uuid& uuid)
{
  std::array<char, 40> buffer;
  auto size = snprintf(buffer.data(), buffer.size(), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    uuid.time_low, uuid.time_mid, uuid.time_hi_and_version, uuid.clk_seq_hi_res, uuid.clk_seq_low,
    uuid.node[0], uuid.node[1], uuid.node[2], uuid.node[3], uuid.node[4], uuid.node[5]);
  if (size < 0 || size >= buffer.size()) {
    throw std::runtime_error("uuid: buffer error");
  }
  buffer[size] = '\0';
  return os << buffer.data();
}

}  // namespace ice