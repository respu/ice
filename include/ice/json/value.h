#pragma once
#include <ice/json/data.h>
#include <ice/json/exception.h>
#include <ice/json/types.h>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <string>
#include <cstdint>

namespace ice {
namespace json {

// ECMA-404 The JSON Data Interchange Standard implementation.
class value {
public:
  // Constructs a 'null' json value.
  value() = default;

  value(value&& other);
  value(const value& other);

  // Constructs a json value from the given parameter.
  // In case of a non-basic json value type template parameter, the user-provided 'ice::json_traits' are used.
  template <typename T>
  explicit value(T&& value)
  {
    json_traits<std::remove_reference_t<T>>::assign(*this, std::forward<T>(value));
  }

  // Constructs a new array or object from an initializer list.
  // Usage: ice::json::value root = { 1, "two", "three", 4 };  // array
  // Usage: ice::json::value root = { { 1, "two" }, { "three", 4 } };  // object
  value(std::initializer_list<value> list);

  ~value();

  value& operator=(value&& other);
  value& operator=(const value& other);

  // Assigns a new json value from the given parameter.
  // In case of a non-basic json value type template parameter, the user-provided 'ice::json_traits' are used.
  template <typename T>
  value& operator=(T&& value)
  {
    clear();
    json_traits<std::remove_reference_t<T>>::assign(*this, std::forward<T>(value));
    return *this;
  }

  // Returns the basic value type.
  json::type type() const;

  // Checks if the basic json value type matches the given template parameter type.
  // In case of a non-basic json value type template parameter, the user-provided 'ice::json_traits' are used.
  template <typename T>
  bool is() const
  {
    return json_traits<T>::is(*this);
  }

  // Returns a reference to the underlying basic value.
  // Throws 'ice::json::type_error' in case of a non-basic json value type.
  template <typename T>
  T& data()
  {
    return json_traits<T>::data(*this);
  }

  // Returns a const reference to the underlying basic value.
  // Throws 'ice::json::type_error' in case of a non-basic json value type.
  template <typename T>
  const T& data() const
  {
    return json_traits<T>::data(*this);
  }

  // Converts the underlying basic value to the provided template parameter value type.
  // In case of a non-basic json value type template parameter, the user-provided 'ice::json_traits' are used.
  template <typename T>
  T as() const
  {
    return json_traits<T>::as(*this);
  }

  // Returns true if the json value is an empty array, an empty object or any other json value type.
  bool empty() const noexcept;

  // Returns the number of elements in the json array or object.
  std::size_t size() const noexcept;

  // Clears the underlying string, array or object json value.
  void clear() noexcept;

  // Initializes an array and appends a new element.
  // In case of a non-basic json value type template parameter, the user-provided 'ice::json_traits' are used.
  template <typename T>
  void append(T&& value)
  {
    if (type_ != json::type::array) {
      reset(json::type::array);
    }
    return data_.array.push_back(std::forward<T>(value));
  }

  // Returns a reference to the requested array element.
  // Throws 'ice::json::type_error' if the json value is not an array.
  // Throws 'ice::json::range_error' if the given index is out of range.
  value& operator[](std::size_t index);

  // Returns a const reference to the requested array element.
  // Throws 'ice::json::type_error' if the json value is not an array.
  // Throws 'ice::json::range_error' if the given index is out of range.
  const value& operator[](std::size_t index) const;

  // Erases an array element.
  // Throws 'ice::json::type_error' if the json value is not an array.
  // Throws 'ice::json::range_error' if the given index is out of range.
  void erase(std::size_t index);

  // Initializes an object and returns a reference to the requested element value.
  value& operator[](const std::string& key);

  // Returns a reference to the requested element value.
  // Throws 'ice::json::type_error' if the json value is not an object.
  // Throws 'ice::json::range_error' if the given key is not found.
  const value& operator[](const std::string& key) const;

  // Returns an iterator to the requested object element.
  // Returns 'end()' if the given key is not found or the json value is not an object.
  iterator find(const std::string& key) noexcept;

  // Returns a const iterator to the requested object element.
  // Returns 'end()' if the given key is not found or the json value is not an object.
  const_iterator find(const std::string& key) const noexcept;

  // Erases an object element.
  // Returns 'end()' if the given key is not found or the json value is not an object.
  iterator erase(const std::string& key) noexcept;

  // Returns an iterator to the first element of an array or object json value.
  iterator begin() noexcept;

  // Returns a const iterator to the first element of an array or object json value.
  const_iterator begin() const noexcept;

  // Returns a const iterator to the first element of an array or object json value.
  const_iterator cbegin() const noexcept;

  // Returns an iterator to the last element of an array or object json value.
  iterator end() noexcept;

  // Returns a const iterator to the last element of an array or object json value.
  const_iterator end() const noexcept;

  // Returns a const iterator to the last element of an array or object json value.
  const_iterator cend() const noexcept;

  // Returns a reverse iterator to the last element of an array or object json value.
  reverse_iterator rbegin() noexcept;

  // Returns a const reverse iterator to the last element of an array or object json value.
  const_reverse_iterator rbegin() const noexcept;

  // Returns a const reverse iterator to the last element of an array or object json value.
  const_reverse_iterator crbegin() const noexcept;

  // Returns a reverse iterator to the first element of an array or object json value.
  reverse_iterator rend() noexcept;

  // Returns a const reverse iterator to the first element of an array or object json value.
  const_reverse_iterator rend() const noexcept;

  // Returns a const reverse iterator to the first element of an array or object json value.
  const_reverse_iterator crend() const noexcept;

  // Erases an array or object element.
  iterator erase(iterator it) noexcept;

  // Erases an array or object element.
  iterator erase(const_iterator it) noexcept;

  // Erases an array or object element range.
  iterator erase(iterator first, iterator last) noexcept;

  // Erases an array or object element range.
  iterator erase(const_iterator first, const_iterator last) noexcept;

  // Resets the json value type to 'null'.
  void reset();

  // Resets the json value type and initializes with a default value.
  // boolean: false
  // number: 0.0
  // string: ""
  void reset(json::type type);

  // Resets the json value type to 'boolean' and assigns the given value.
  void reset(boolean v);

  // Resets the json value type to 'number' and assigns the given value.
  void reset(number v);

  // Resets the json value type to 'string' and assigns the given value.
  void reset(string v);

  // Resets the json value type to 'array' and assigns the given value.
  void reset(array v);

  // Resets the json value type to 'object' and assigns the given value.
  void reset(object v);

  // Returns the json value as a boolean.
  boolean as_boolean() const noexcept;

  // Returns the json value as a number.
  number as_number() const noexcept;

  // Returns the json value as a string.
  string as_string() const noexcept;

private:
  json::data data_;
  json::type type_ = json::type::null;

  friend struct json_traits<json::type>;
  friend struct json_traits<json::null>;
  friend struct json_traits<json::boolean>;
  friend struct json_traits<json::number>;
  friend struct json_traits<json::string>;
  friend struct json_traits<json::array>;
  friend struct json_traits<json::object>;
  friend std::ostream& format(std::ostream& os, const value& root, bool pretty, std::size_t offset);
};

bool operator==(const value& a, const value& b);
bool operator!=(const value& a, const value& b);
bool operator<(const value& a, const value& b);
bool operator>(const value& a, const value& b);
bool operator<=(const value& a, const value& b);
bool operator>=(const value& a, const value& b);

std::ostream& operator<<(std::ostream& os, const value& root);

inline array::array(std::initializer_list<value> list)
{
  for (const auto& e : list) {
    emplace_back(e);
  }
}

inline object::object(std::initializer_list<std::initializer_list<value>> list)
{
  for (const auto& e : list) {
    if (e.size() != 2) {
      throw type_error("object initializer list size error");
    }
    emplace_back(e.begin()->as_string(), *(e.begin() + 1));
  }
}

}  // namespace json
}  // namespace ice