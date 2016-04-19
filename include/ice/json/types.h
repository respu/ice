#pragma once
#include <ice/optional.h>
#include <initializer_list>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace ice {

// Used by 'ice::json::value' to convert a json value to 'T' and 'T' to a json value.
template <typename T>
struct json_traits;

namespace json {

enum struct type {
  null, boolean, number, string, array, object
};

std::ostream& operator<<(std::ostream& os, json::type type);

// Json null value.
struct null {};

inline bool operator==(const null&, const null&)
{
  return true;
}

inline bool operator!=(const null&, const null&)
{
  return false;
}

inline bool operator<(const null&, const null&)
{
  return false;
}

inline bool operator>(const null&, const null&)
{
  return false;
}

inline bool operator<=(const null&, const null&)
{
  return true;
}

inline bool operator>=(const null&, const null&)
{
  return true;
}

// Json boolean value.
using boolean = bool;

// Json number value.
using number = double;

// Json string value.
using string = std::string;

// Json array or object element.
template <typename T>
struct element {
  // Json object element name.
  // Should not be set when element is part of an array. Must not be set when element is part of an object.
  ice::optional<std::string> name;

  // Json object element or array element value.
  T value;

  element(T v) : value(std::move(v))
  {}

  element(std::string name, T value) : name(std::move(name)), value(std::move(value))
  {}
};

template <typename T>
inline bool operator==(const element<T>& a, const element<T>& b)
{
  if (a.name && b.name) {
    return a.name == b.name && a.value == b.value;
  }
  return a.value == b.value;
}

template <typename T>
inline bool operator!=(const element<T>& a, const element<T>& b)
{
  return !(a == b);
}

template <typename T>
inline bool operator<(const element<T>& a, const element<T>& b)
{
  if (a.name && b.name) {
    return a.name < b.name;
  }
  return a.value < b.value;
}

template <typename T>
inline bool operator>(const element<T>& a, const element<T>& b)
{
  return !(a < b);
}

template <typename T>
inline bool operator<=(const element<T>& a, const element<T>& b)
{
  return a == b || a < b;
}

template <typename T>
inline bool operator>=(const element<T>& a, const element<T>& b)
{
  return a == b || a > b;
}

class value;

// Used as the base class for both, array and object json values. This allows the use of a single iterator type for both types.
// Thes array and object types cannot be typedefs because they have separate type_traits specializations.
using collection = std::vector<element<value>>;

// Json value iterator.
using iterator = collection::iterator;

// Json value const iterator.
using const_iterator = collection::const_iterator;

// Json value reverse iterator.
using reverse_iterator = collection::reverse_iterator;

// Json value const reverse iterator.
using const_reverse_iterator = collection::const_reverse_iterator;

// Json array value.
class array : public collection {
public:
  array() = default;
  array(array&& other) = default;
  array(const array& other) = default;

  // Constructs a new array from an initializer list.
  // Usage: ice::json::value root = ice::json::array{ 1, "two", "three", 4 };
  explicit array(std::initializer_list<value> list);  // defined in value.h

  array& operator=(array&& other) = default;
  array& operator=(const array& other) = default;
};

// Json object value.
class object : public collection {
public:
  object() = default;
  object(object&& other) = default;
  object(const object& other) = default;

  // Constructs a new object from an initializer list.
  // Usage: ice::json::value root = ice::json::object{ { 1, "two" }, { "three", 4 } };
  explicit object(std::initializer_list<std::initializer_list<value>> list);  // defined in value.cc

  object& operator=(object&& other) = default;
  object& operator=(const object& other) = default;
};

template <typename T>
struct is_numeric {
  static constexpr bool value = std::is_integral<T>::value && !std::is_same<bool, T>::value;
};

// Json number value compatible type.
template <typename T>
using numeric = typename std::enable_if_t<is_numeric<T>::value, T>;

}  // namespace json
}  // namespace ice