#include <ice/uuid.h>
#include <ice/giant.h>
#include <openssl/rand.h>
#include <algorithm>
#include <stdexcept>
#include <cstdio>

// Number of bytes in a formatted UUID string without the terminating character.
#define UUID_FORMAT_SIZE (8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12)

// Format for sscanf and snprintf.
#define UUID_FORMAT "%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"

// Number of elements that have to be parsed/formatted by sscanf and snprintf.
#define UUID_FORMAT_COUNT 11

namespace ice {

uuid::uuid(const std::string& str)
{
  auto count = std::sscanf(str.c_str(), UUID_FORMAT,
    &tl, &tm, &thv, &csr, &csl, &n[0], &n[1], &n[2], &n[3], &n[4], &n[5]);
  if (count != UUID_FORMAT_COUNT) {
    throw std::runtime_error("uuid: format error");
  }
}

uuid::uuid(const data_type& data)
{
  auto beg = reinterpret_cast<std::uint8_t*>(this);
  std::copy(data.begin(), data.end(), beg);
  tl  = giant::letoh(tl);
  tm  = giant::letoh(tm);
  thv = giant::letoh(thv);
}

uuid::uuid(const std::uint8_t* data, std::size_t size)
{
  if (size != 16) {
    throw std::runtime_error("uuid: blob size error");
  }
  auto beg = reinterpret_cast<std::uint8_t*>(this);
  std::copy(data, data + size, beg);
  tl = giant::letoh(tl);
  tm = giant::letoh(tm);
  thv = giant::letoh(thv);
}

std::string uuid::str() const
{
  std::string str;
  str.resize(UUID_FORMAT_SIZE + 1);
  auto size = std::snprintf(&str[0], UUID_FORMAT_SIZE + 1, UUID_FORMAT,
    tl, tm, thv, csr, csl, n[0], n[1], n[2], n[3], n[4], n[5]);
  if (size != UUID_FORMAT_SIZE) {
    throw std::runtime_error("uuid: format size error");
  }
  str.resize(static_cast<std::size_t>(size));
  return str;
}

uuid::data_type uuid::data() const
{
  auto uuid = *this;
  uuid.tl  = giant::htole(uuid.tl);
  uuid.tm  = giant::htole(uuid.tm);
  uuid.thv = giant::htole(uuid.thv);
  data_type data;
  auto beg = reinterpret_cast<const std::uint8_t*>(&uuid);
  std::copy(beg, beg + data.size(), data.begin());
  return data;
}

uuid uuid::generate()
{
  ice::uuid uuid;
  if (!RAND_bytes(reinterpret_cast<std::uint8_t*>(&uuid), sizeof(uuid))) {
    throw std::runtime_error("uuid: random bytes error");
  }

  // Set the UUID version according to RFC-4122 (Section 4.2).
  uuid.thv = static_cast<decltype(uuid.thv)>((uuid.thv & 0x0FFF) | 0x4000);
  uuid.csr = static_cast<decltype(uuid.csr)>((uuid.csr & 0x3F) | 0x80);

  return uuid;
}

bool uuid::check(const std::string& str)
{
  if (str.size() != 36) {
    return false;
  }
  for (std::size_t i = 0, size = str.size(); i < size; i++) {
    auto c = str[i];
    switch (i) {
    case 8:
    case 13:
    case 18:
    case 23:
      if (c != '-') {
        return false;
      }
      break;
    default:
      if ((c < '0' || c > '9') && (c < 'a' || c > 'f')) {
        return false;
      }
      break;
    }
  }
  return true;
}

}  // namespace ice
