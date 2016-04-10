#include <ice/archive/inflate.h>
#include <zlib.h>
#include <array>
#include <limits>
#include <stdexcept>

namespace ice {
namespace archive {

struct inflate::stream : public z_stream {};

inflate::inflate(inflate::format format, int window_bits) :
  stream_(std::make_unique<stream>())
{
  if (window_bits < 8 || window_bits > 15) {
    throw std::invalid_argument("zlib: invalid inflate window size");
  }
  switch (format) {
  case inflate::format::inflate: window_bits = -window_bits; break;
  case inflate::format::zlib: break;
  default: throw std::invalid_argument("zlib: invalid inflate format"); break;
  }
  if (inflateInit2(stream_.get(), window_bits) != Z_OK) {
    throw std::runtime_error("zlib: inflate init error");
  }
}

inflate::inflate(inflate&& other) :
  inflate()
{
  stream_.swap(other.stream_);
}

inflate& inflate::operator=(inflate&& other)
{
  stream_.swap(other.stream_);
  return *this;
}

inflate::~inflate()
{
  inflateEnd(stream_.get());
}

void inflate::process(const std::uint8_t* data, std::size_t size, bool finish, inflate::handler handler)
{
  using data_type = decltype(stream_->next_in);
  using size_type = decltype(stream_->avail_in);

  if (size == 0 && !finish) {
    return;
  }

  if (size > 0 && !data) {
    throw std::invalid_argument("zlib: invalid inflate data");
  }

  if (size > std::numeric_limits<size_type>::max()) {
    throw std::invalid_argument("zlib: invalid inflate size");
  }

  if (finished_) {
    reset();
  }

  stream_->next_in = reinterpret_cast<data_type>(const_cast<std::uint8_t*>(data));
  stream_->avail_in = static_cast<size_type>(size);

  int ret = Z_OK;
  std::array<std::uint8_t, 8192> buffer;
  do {
    stream_->next_out = reinterpret_cast<data_type>(buffer.data());
    stream_->avail_out = static_cast<size_type>(buffer.size());

    ret = ::inflate(stream_.get(), finish ? Z_FINISH : Z_NO_FLUSH);
    auto inflate_size = buffer.size() - stream_->avail_out;

    switch (ret) {
    case Z_OK: break;
    case Z_STREAM_END: break;
    case Z_STREAM_ERROR:
      throw std::runtime_error("zlib: inflate stream error");
      break;
    case Z_NEED_DICT:
      throw std::runtime_error("zlib: inflate dict error");
      break;
    case Z_DATA_ERROR:
      throw std::runtime_error("zlib: inflate data error");
      break;
    case Z_MEM_ERROR:
      throw std::runtime_error("zlib: inflate memory error");
      break;
    case Z_BUF_ERROR:
      if (!finish) {
        return;
      }
      throw std::runtime_error("zlib: inflate buffer error");
      break;
    default:
      throw std::runtime_error("zlib: unknown inflate error");
      break;
    }

    if (inflate_size > 0) {
      handler(buffer.data(), inflate_size);
    }
  } while (ret != Z_STREAM_END && stream_->avail_in > 0);

  finished_ = finish && ret == Z_STREAM_END;
}

void inflate::reset()
{
  if (finished_) {
    if (inflateReset(stream_.get()) != Z_OK) {
      throw std::runtime_error("zlib: inflate reset error");
    }
    finished_ = false;
  }
}

}  // namespace archive
}  // namespace ice