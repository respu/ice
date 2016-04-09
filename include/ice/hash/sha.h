#pragma once
#include <ice/hash/base.h>
#include <ice/string_view.h>
#include <array>
#include <memory>
#include <cstdint>

namespace ice {
namespace hash {

enum class sha_size : std::size_t {
  sha1   = 20,
  sha224 = 28,
  sha256 = 32,
  sha384 = 48,
  sha512 = 64
};

template<sha_size size>
using sha_type = std::array<std::uint8_t, static_cast<std::size_t>(size)>;

template<sha_size size>
class sha : public base<sha_type<size>, sha<size>> {
public:
  using value_type = sha_type<size>;

  sha();
  ~sha();

  void append(const std::uint8_t* data, std::size_t size);

  value_type value() const;
  void reset();

private:
  struct context;
  std::unique_ptr<context> context_;
};

using sha1   = sha<sha_size::sha1>;
using sha224 = sha<sha_size::sha224>;
using sha256 = sha<sha_size::sha256>;
using sha384 = sha<sha_size::sha384>;
using sha512 = sha<sha_size::sha512>;

}  // namespace hash
}  // namespace ice