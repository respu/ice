#include <ice/hash/sha.h>
#include <openssl/sha.h>

namespace ice {
namespace hash {

// SHA1 =======================================================================

template<>
struct sha<sha_size::sha1>::context : public SHA_CTX {};

sha<sha_size::sha1>::sha() :
  context_(std::make_unique<context>())
{
  reset();
}

template<>
sha<sha_size::sha1>::~sha()
{}

template<>
void sha<sha_size::sha1>::append(const std::uint8_t* data, std::size_t size)
{
  SHA1_Update(context_.get(), data, size);
}

template<>
sha<sha_size::sha1>::value_type sha<sha_size::sha1>::value() const
{
  value_type value;
  SHA1_Final(value.data(), context_.get());
  return value;
}

template<>
void sha<sha_size::sha1>::reset()
{
  SHA1_Init(context_.get());
}

// SHA224 =====================================================================

template<>
struct sha<sha_size::sha224>::context : public SHA256_CTX {};

sha<sha_size::sha224>::sha() :
  context_(std::make_unique<context>())
{
  reset();
}

template<>
sha<sha_size::sha224>::~sha()
{}

template<>
void sha<sha_size::sha224>::append(const std::uint8_t* data, std::size_t size)
{
  SHA224_Update(context_.get(), data, size);
}

template<>
sha<sha_size::sha224>::value_type sha<sha_size::sha224>::value() const
{
  value_type value;
  SHA224_Final(value.data(), context_.get());
  return value;
}

template<>
void sha<sha_size::sha224>::reset()
{
  SHA224_Init(context_.get());
}

// SHA256 =====================================================================

template<>
struct sha<sha_size::sha256>::context : public SHA256_CTX {};

sha<sha_size::sha256>::sha() :
  context_(std::make_unique<context>())
{
  reset();
}

template<>
sha<sha_size::sha256>::~sha()
{}

template<>
void sha<sha_size::sha256>::append(const std::uint8_t* data, std::size_t size)
{
  SHA256_Update(context_.get(), data, size);
}

template<>
sha<sha_size::sha256>::value_type sha<sha_size::sha256>::value() const
{
  value_type value;
  SHA256_Final(value.data(), context_.get());
  return value;
}

template<>
void sha<sha_size::sha256>::reset()
{
  SHA256_Init(context_.get());
}

// SHA384 =====================================================================

template<>
struct sha<sha_size::sha384>::context : public SHA512_CTX {};

sha<sha_size::sha384>::sha() :
  context_(std::make_unique<context>())
{
  reset();
}

template<>
sha<sha_size::sha384>::~sha()
{}

template<>
void sha<sha_size::sha384>::append(const std::uint8_t* data, std::size_t size)
{
  SHA384_Update(context_.get(), data, size);
}

template<>
sha<sha_size::sha384>::value_type sha<sha_size::sha384>::value() const
{
  value_type value;
  SHA384_Final(value.data(), context_.get());
  return value;
}

template<>
void sha<sha_size::sha384>::reset()
{
  SHA384_Init(context_.get());
}

// SHA512 =====================================================================

template<>
struct sha<sha_size::sha512>::context : public SHA512_CTX {};

sha<sha_size::sha512>::sha() :
  context_(std::make_unique<context>())
{
  reset();
}

template<>
sha<sha_size::sha512>::~sha()
{}

template<>
void sha<sha_size::sha512>::append(const std::uint8_t* data, std::size_t size)
{
  SHA512_Update(context_.get(), data, size);
}

template<>
sha<sha_size::sha512>::value_type sha<sha_size::sha512>::value() const
{
  value_type value;
  SHA512_Final(value.data(), context_.get());
  return value;
}

template<>
void sha<sha_size::sha512>::reset()
{
  SHA512_Init(context_.get());
}

}  // namespace hash
}  // namespace ice