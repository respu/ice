#pragma once
#include <functional>
#include <memory>
#include <cstdint>

namespace ice {
namespace archive {

class deflate {
public:
  // Handler for compressed data.
  using handler = std::function<void(const std::uint8_t* data, std::size_t size)>;

  // Compression format.
  enum class format {
    deflate,  // raw deflate without a header or trailer
    zlib      // zlib header and trailer
  };

  // Initializes the zlib deflate stream.
  // format       : The compression format.
  // level        : Compression level. Must be between 0 and 9.
  // window_bits  : Window size. Larger values result in better compression. Must be between 8 and 15.
  // memory_level : Memory level. Larger values result in better performance. Must be between 1 and 9.
  deflate(deflate::format format = deflate::format::deflate, int level = -1, int window_bits = 15, int memory_level = 8);

  deflate(deflate&& other);
  deflate& operator=(deflate&& other);

  ~deflate();

  // Processes data.
  void process(const std::uint8_t* data, std::size_t size, bool finish, deflate::handler handler);

  // Processes data.
  template<typename T, typename Handler>
  std::enable_if_t<!std::is_same<T, std::uint8_t>::value, void> process(const T* data, std::size_t size, bool finish, Handler&& handler)
  {
    process(reinterpret_cast<const std::uint8_t*>(data), sizeof(T) * size, finish, std::forward<Handler>(handler));
  }

  // Finishes data processing.
  template<typename Handler>
  void finish(Handler&& handler)
  {
    process(nullptr, 0, true, std::forward<Handler>(handler));
  }

  // Resets the stream without changing the options.
  void reset();

private:
  struct stream;
  std::unique_ptr<stream> stream_;
  bool finished_ = false;
};

}  // namespace archive
}  // namespace ice