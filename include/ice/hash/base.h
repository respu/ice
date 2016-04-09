#pragma once
#include <initializer_list>
#include <utility>
#include <type_traits>
#include <vector>
#include <cstdint>

namespace ice {
namespace hash {

template<typename Type, typename Impl>
class base {
public:
  using value_type = Type;

  virtual ~base() = default;

  // Modifies the hash value with the string literal elements.
  template<std::size_t N>
  void process(const char(&STR)[N])
  {
    static_cast<Impl*>(this)->append(reinterpret_cast<const std::uint8_t*>(STR), N - 1);
  }

  // Modifies the hash value with the sequence elements.
  template<typename Sequence, typename = std::enable_if_t<std::is_integral<typename Sequence::value_type>::value, Sequence>>
  void process(const Sequence& sequence)
  {
    static_cast<Impl*>(this)->append(reinterpret_cast<const std::uint8_t*>(sequence.data()), sizeof(*sequence.data()) * sequence.size());
  }

  // Modifies the hash value with the initializer list elements.
  template<typename T, typename = std::enable_if_t<std::is_integral<T>::value, T>>
  void process(std::initializer_list<T> list)
  {
    std::vector<T> buffer(list.begin(), list.end());
    static_cast<Impl*>(this)->append(reinterpret_cast<const std::uint8_t*>(buffer.data()), sizeof(T) * buffer.size());
  }

  // Modifies the hash value with the given bytes.
  template<typename T, typename = std::enable_if_t<std::is_integral<T>::value, T>>
  void process(const T* data, std::size_t size)
  {
    static_cast<Impl*>(this)->append(reinterpret_cast<const std::uint8_t*>(data), sizeof(T) * size);
  }

  // Hashes the string literal.
  template<std::size_t N>
  static value_type hash(const char(&STR)[N])
  {
    Impl impl;
    impl.process(STR);
    return impl.value();
  }

  // Hashes the sequence elements.
  template<typename Sequence>
  static value_type hash(const Sequence& sequence)
  {
    Impl impl;
    impl.process(sequence);
    return impl.value();
  }

  // Hashes the initializer list elements.
  template<typename T>
  static value_type hash(std::initializer_list<T> list)
  {
    Impl impl;
    impl.process(list);
    return impl.value();
  }

  // Hashes the given bytes.
  template<typename T>
  static value_type hash(const T* data, std::size_t size)
  {
    Impl impl;
    impl.process(data, size);
    return impl.value();
  }
};

}  // namespace hash
}  // namespace ice