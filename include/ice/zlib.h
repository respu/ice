#pragma once
#include <functional>
#include <memory>
#include <cstdint>

namespace ice {
namespace zlib {

struct stream;

enum class format {
  deflate,  // raw deflate without a header or trailer
  gzip,     // gzip header and trailer
  zlib      // zlib header and trailer
};

using handler = std::function<void(const std::uint8_t* data, std::size_t size)>;

class inflate {
public:
  // Initializes the zlib inflate stream.
  // format       : The compression format.
  // window_bits  : Window size. Larger values result in faster inflation. Must be between 8 and 15.
  inflate(zlib::format format = zlib::format::deflate, int window_bits = 15);

  inflate(inflate&& other);
  inflate& operator=(inflate&& other);

  ~inflate();

  void process(const std::uint8_t* data, std::size_t size, bool finish, zlib::handler handler);

  template<typename T, typename Handler>
  std::enable_if_t<!std::is_same<T, std::uint8_t>::value, void> process(const T* data, std::size_t size, bool finish, Handler&& handler)
  {
    process(reinterpret_cast<const std::uint8_t*>(data), sizeof(T) * size, finish, std::forward<Handler>(handler));
  }

  template<typename Handler>
  void finish(Handler&& handler)
  {
    process(nullptr, 0, true, std::forward<Handler>(handler));
  }

  void reset();

private:
  std::unique_ptr<stream> stream_;
  bool finished_ = false;
};

class deflate {
public:
  // Initializes the zlib deflate stream.
  // format       : The compression format.
  // level        : Compression level. Must be between 0 and 9.
  // window_bits  : Window size. Larger values result in better compression. Must be between 8 and 15.
  // memory_level : Memory level. Larger values result in better performance. Must be between 1 and 9.
  deflate(zlib::format format = zlib::format::deflate, int level = -1, int window_bits = 15, int memory_level = 8);

  deflate(deflate&& other);
  deflate& operator=(deflate&& other);

  ~deflate();

  void process(const std::uint8_t* data, std::size_t size, bool finish, zlib::handler handler);

  template<typename T, typename Handler>
  std::enable_if_t<!std::is_same<T, std::uint8_t>::value, void> process(const T* data, std::size_t size, bool finish, Handler&& handler)
  {
    process(reinterpret_cast<const std::uint8_t*>(data), sizeof(T) * size, finish, std::forward<Handler>(handler));
  }

  template<typename Handler>
  void finish(Handler&& handler)
  {
    process(nullptr, 0, true, std::forward<Handler>(handler));
  }

  void reset();

private:
  std::unique_ptr<stream> stream_;
  bool finished_ = false;
};

}  // namespace zlib
}  // namespace ice