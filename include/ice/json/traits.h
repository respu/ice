#pragma once
#include <ice/json/value.h>
#include <ice/utf8.h>
#include <map>
#include <sstream>
#include <vector>
#include <cstdint>

namespace ice {

// Default 'ice::json_traits' implementation with fallback function for natively supported types.
template <typename T>
struct json_traits {
  // Safely converts a json number value compatible type to a json number.
  static void assign(json::value& self, const json::numeric<T>& v)
  {
    auto number_v = static_cast<json::number>(v);
    if (v != static_cast<T>(number_v)) {
      throw json::bad_cast::numeric<T>(v);
    }
    self = number_v;
  }

  // Checks if the json value can be converted to a json number value compatible type.
  static bool is(const json::value& self, json::numeric<T>* dummy = nullptr)
  {
    return self.type_ == json::type::number;
  }

  // Converts the json value to a json number value compatible type.
  static T as(const json::value& self, json::numeric<T>* dummy = nullptr)
  {
    auto number_v = self.as<json::number>();
    auto v = static_cast<T>(number_v);
    if (number_v != static_cast<json::number>(v)) {
      throw json::bad_cast::numeric<T>(number_v);
    }
    return v;
  }
};

// Makes it possible to reset the json value by assigning it a new type.
template <>
struct json_traits<json::type> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::type& v)
  {
    self.reset(v);
  }
};

// Makes the 'null' json value type accessible through public APIs.
template <>
struct json_traits<json::null> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::null& v)
  {
    self.reset();
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type_ == json::type::null;
  }

  // Used by 'ice::json::value::data()'. Should not be implemented for non-native json value types.
  static json::null& data(json::value& self)
  {
    if (self.type_ != json::type::null) {
      throw json::type_error::data(self.type_, json::type::null);
    }
    return self.data_.null;
  }

  // Used by 'const ice::json::value::data() const'. Should not be implemented for non-native json value types.
  static const json::null& data(const json::value& self)
  {
    if (self.type_ != json::type::null) {
      throw json::type_error::const_data(self.type_, json::type::null);
    }
    return self.data_.null;
  }

  // Used by 'ice::json::value::as()'.
  static json::null as(const json::value& self)
  {
    return json::null();
  }
};

// Makes the 'boolean' json value type accessible through public APIs.
template <>
struct json_traits<json::boolean> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::boolean& v)
  {
    self.reset(v);
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type_ == json::type::boolean;
  }

  // Used by 'ice::json::value::data()'. Should not be implemented for non-native json value types.
  static json::boolean& data(json::value& self)
  {
    if (self.type_ != json::type::boolean) {
      throw json::type_error::data(self.type_, json::type::boolean);
    }
    return self.data_.boolean;
  }

  // Used by 'const ice::json::value::data() const'. Should not be implemented for non-native json value types.
  static const json::boolean& data(const json::value& self)
  {
    if (self.type_ != json::type::boolean) {
      throw json::type_error::const_data(self.type_, json::type::boolean);
    }
    return self.data_.boolean;
  }

  // Used by 'ice::json::value::as()'.
  static json::boolean as(const json::value& self)
  {
    return self.as_boolean();
  }
};

// Makes the 'number' json value type accessible through public APIs.
template <>
struct json_traits<json::number> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::number& v)
  {
    self.reset(v);
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type_ == json::type::number;
  }

  // Used by 'ice::json::value::data()'. Should not be implemented for non-native json value types.
  static json::number& data(json::value& self)
  {
    if (self.type_ != json::type::number) {
      throw json::type_error::data(self.type_, json::type::number);
    }
    return self.data_.number;
  }

  // Used by 'const ice::json::value::data() const'. Should not be implemented for non-native json value types.
  static const json::number& data(const json::value& self)
  {
    if (self.type_ != json::type::number) {
      throw json::type_error::const_data(self.type_, json::type::number);
    }
    return self.data_.number;
  }

  // Used by 'ice::json::value::as()'.
  static json::number as(const json::value& self)
  {
    return self.as_number();
  }
};

// Makes the 'string' json value type accessible through public APIs.
template <>
struct json_traits<json::string> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, json::string&& v)
  {
    self.reset(std::move(v));
  }

  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::string& v)
  {
    self.reset(v);
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type_ == json::type::string;
  }

  // Used by 'ice::json::value::data()'. Should not be implemented for non-native json value types.
  static json::string& data(json::value& self)
  {
    if (self.type_ != json::type::string) {
      throw json::type_error::data(self.type_, json::type::string);
    }
    return self.data_.string;
  }

  // Used by 'const ice::json::value::data() const'. Should not be implemented for non-native json value types.
  static const json::string& data(const json::value& self)
  {
    if (self.type_ != json::type::string) {
      throw json::type_error::const_data(self.type_, json::type::string);
    }
    return self.data_.string;
  }

  // Used by 'ice::json::value::as()'.
  static json::string as(const json::value& self)
  {
    return self.as_string();
  }
};

// Makes it possible to check for the 'array' json value type.
template <>
struct json_traits<json::array> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, json::array&& v)
  {
    self.reset(std::move(v));
  }

  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::array& v)
  {
    self.reset(v);
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type_ == json::type::array;
  }

  // Used by 'ice::json::value::data()'. Should not be implemented for non-native json value types.
  static json::array& data(json::value& self)
  {
    if (self.type_ != json::type::array) {
      throw json::type_error::data(self.type_, json::type::array);
    }
    return self.data_.array;
  }

  // Used by 'const ice::json::value::data() const'. Should not be implemented for non-native json value types.
  static const json::array& data(const json::value& self)
  {
    if (self.type_ != json::type::array) {
      throw json::type_error::const_data(self.type_, json::type::array);
    }
    return self.data_.array;
  }

  // Used by 'ice::json::value::as()'.
  static json::array as(const json::value& self)
  {
    return data(self);
  }
};

// Makes it possible to check for the 'object' json value type.
template <>
struct json_traits<json::object> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, json::object&& v)
  {
    self.reset(std::move(v));
  }

  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const json::object& v)
  {
    self.reset(v);
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type_ == json::type::object;
  }

  // Used by 'ice::json::value::data()'. Should not be implemented for non-native json value types.
  static json::object& data(json::value& self)
  {
    if (self.type_ != json::type::object) {
      throw json::type_error::data(self.type_, json::type::object);
    }
    return self.data_.object;
  }

  // Used by 'const ice::json::value::data() const'. Should not be implemented for non-native json value types.
  static const json::object& data(const json::value& self)
  {
    if (self.type_ != json::type::object) {
      throw json::type_error::const_data(self.type_, json::type::object);
    }
    return self.data_.object;
  }

  // Used by 'ice::json::value::as()'.
  static json::object as(const json::value& self)
  {
    return data(self);
  }
};

// Adds support for 'float' (de)serialization.
template <>
struct json_traits<float> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, float v)
  {
    self.reset(static_cast<double>(v));
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type() == json::type::number;
  }

  // Used by 'ice::json::value::as()'.
  static float as(const json::value& self)
  {
    return static_cast<float>(self.as_number());
  }
};

// Adds support for 'const char*' deserialization.
template <>
struct json_traits<const char*> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const char* v)
  {
    self.reset(ice::json::string(v ? v : ""));
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type() == json::type::string;
  }
};

// Adds support for 'std::vector<T>' (de)serialization.
template <typename T>
struct json_traits<std::vector<T>> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, std::vector<T>&& v)
  {
    self.reset(json::type::array);
    for (auto& e : v) {
      self.append(std::move(e));
    }
  }

  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const std::vector<T>& v)
  {
    self.reset(json::type::array);
    for (auto& e : v) {
      self.append(e);
    }
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type() == json::type::array;
  }

  // Used by 'ice::json::value::as()'.
  static std::vector<T> as(const json::value& self)
  {
    std::vector<T> v;
    if (self.type() == json::type::array) {
      for (const auto& e : self) {
        v.push_back(e.value.as<T>());
      }
    }
    return v;
  }
};

// Adds support for 'std::map<std::string, T>' (de)serialization.
template <typename T>
struct json_traits<std::map<std::string, T>> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, std::map<std::string, T>&& v)
  {
    self.reset(json::type::object);
    for (auto& e : v) {
      self[e.first] = std::move(e.second);
    }
  }

  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const std::map<std::string, T>& v)
  {
    self.reset(json::type::object);
    for (auto& e : v) {
      self[e.first] = e.second;
    }
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type() == json::type::object;
  }

  // Used by 'ice::json::value::as()'.
  static std::map<std::string, T> as(const json::value& self)
  {
    std::map<std::string, T> v;
    auto type = self.type();
    if (type == json::type::object) {
      for (const auto& e : self) {
        v[e.name.value()] = e.value.as<T>();
      }
    } else if (type == json::type::array) {
      // Uses the vector index as an object key.
      std::size_t index = 0;
      std::ostringstream oss;
      for (const auto& e : self) {
        oss << index++;
        v[oss.str()] = e.value.as<T>();
        oss.str(std::string());
        oss.clear();
      }
    }
    return v;
  }
};

// Adds support for 'ice::optional<T>' (de)serialization.
template <typename T>
struct json_traits<ice::optional<T>> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, ice::optional<T>&& v)
  {
    if (!v) {
      self.reset();
      return;
    }
    self = std::move(v.value());
  }

  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, const ice::optional<T>& v)
  {
    if (!v) {
      self.reset();
      return;
    }
    self = v.value();
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.type() == json::type::null || self.is<T>();
  }

  // Used by 'ice::json::value::as()'.
  static ice::optional<T> as(const json::value& self)
  {
    if (self.type() == json::type::null) {
      return ice::optional<T>();
    }
    return self.as<T>();
  }
};

}  // namespace ice