#pragma once
#include <ice/json/types.h>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ice {
namespace json {

class type_error : public std::runtime_error {
public:
  type_error(const std::string& message) : std::runtime_error("json type error: " + message)
  {}

  static type_error access(json::type type, std::size_t index)
  {
    std::ostringstream oss;
    oss << "could not access " << type << " index " << index;
    return oss.str();
  }

  static type_error const_access(json::type type, std::size_t index)
  {
    std::ostringstream oss;
    oss << "could not access const " << type << " index " << index;
    return oss.str();
  }

  static type_error access(json::type type, const std::string& key)
  {
    std::ostringstream oss;
    oss << "could not access " << type << " key \"" << key << "\"";
    return oss.str();
  }

  static type_error const_access(json::type type, const std::string& key)
  {
    std::ostringstream oss;
    oss << "could not access const " << type << " key \"" << key << "\"";
    return oss.str();
  }

  static type_error data(json::type type, json::type target)
  {
    std::ostringstream oss;
    oss << "could not get " << type << " data as " << target;
    return oss.str();
  }

  static type_error const_data(json::type type, json::type target)
  {
    std::ostringstream oss;
    oss << "could not get const " << type << " data as " << target;
    return oss.str();
  }

  static type_error as(json::type type, json::type target)
  {
    std::ostringstream oss;
    oss << "could not convert const " << type << " to " << target;
    return oss.str();
  }

  static type_error erase(json::type type, std::size_t index)
  {
    std::ostringstream oss;
    oss << "could not erase " << type << " index " << index;
    return oss.str();
  }

  static type_error erase(json::type type, const std::string& key)
  {
    std::ostringstream oss;
    oss << "could not erase " << type << " key \"" << key << "\"";
    return oss.str();
  }
};

class range_error : public std::range_error {
public:
  range_error(const std::string& message) : std::range_error("json range error: " + message)
  {}

  static range_error access(std::size_t index)
  {
    std::ostringstream oss;
    oss << "array index " << index << " out of range";
    return oss.str();
  }

  static range_error const_access(std::size_t index)
  {
    std::ostringstream oss;
    oss << "const array index " << index << " out of range";
    return oss.str();
  }

  static range_error access(const std::string& key)
  {
    std::ostringstream oss;
    oss << "could not create a new object key \"" << key << "\"";
    return oss.str();
  }

  static range_error const_access(const std::string& key)
  {
    std::ostringstream oss;
    oss << "could not find object key \"" << key << "\"";
    return oss.str();
  }
};

class bad_cast : public std::bad_cast {
public:
  bad_cast(const std::string& message) : message_("json bad cast: " + message)
  {}

  virtual char const* what() const noexcept override
  {
    return message_.c_str();
  }

  template <typename T, typename = json::numeric<T>>
  static bad_cast numeric(T v)
  {
    std::ostringstream oss;
    oss << "could not cast " << v << " to " << type::number;
    return oss.str();
  }

  template <typename T, typename = json::numeric<T>>
  static bad_cast numeric(number v)
  {
    std::ostringstream oss;
    oss << "could not cast " << type::number << " " << v << " to " << typeid(T).name();
    return oss.str();
  }

private:
  std::string message_;
};

class parse_error : public std::runtime_error {
public:
  parse_error(const std::string& message) : std::runtime_error("json parse error: " + message)
  {}
};

}  // namespace json
}  // namespace ice