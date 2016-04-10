#pragma once
#include <functional>
#include <memory>
#include <cstdint>

namespace ice {
namespace archive {

class inflate {
public:
  // Handler for uncompressed data.
  using handler = std::function<void(const std::uint8_t* data, std::size_t size)>;

  // Compression format.
  enum class format {
    inflate,  // raw inflate without a header or trailer
    zlib      // zlib header and trailer
  };

  // Initializes the zlib inflate stream.
  // format       : The compression format.
  // window_bits  : Window size. Larger values result in faster inflation. Must be between 8 and 15.
  inflate(inflate::format format = inflate::format::inflate, int window_bits = 15);

  inflate(inflate&& other);
  inflate& operator=(inflate&& other);

  ~inflate();

  // Processes data.
  void process(const std::uint8_t* data, std::size_t size, bool finish, inflate::handler handler);

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