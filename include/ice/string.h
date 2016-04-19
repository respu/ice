#pragma once
#include <ice/string_view.h>
#include <string>

namespace ice {

constexpr inline bool string_equal(const char* a, const char* b, std::size_t size)
{
  return size == 0 ? true : *a == *b && string_equal(a + 1, b + 1, size - 1);
}

template <std::size_t N>
constexpr inline bool string_equal(const char* a, std::size_t size, const char(&b)[N])
{
  return size == N - 1 ? size > 0 ? string_equal(a, b, size) : true : false;
}

template <std::size_t N>
constexpr inline bool string_equal(const char(&a)[N], const char* b, std::size_t size)
{
  return size == N - 1 ? size > 0 ? string_equal(a, b, size) : true : false;
}

template <size_t N1, size_t N2>
constexpr inline bool string_equal(const char(&a)[N1], const char(&b)[N2])
{
  return N1 == N2 ? N1 - 1 > 0 ? string_equal(a, b, N1 - 1) : true : false;
}

inline void replace(std::string& str, const ice::string_view from, const ice::string_view to)
{
  for (std::size_t pos = 0; (pos = str.find(from.data(), pos, from.size())) != std::string::npos;) {
    str.replace(pos, from.size(), to.data(), to.size());
    pos += to.size();
  }
}

}  // namespace ice