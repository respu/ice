#pragma once
#include <ice/json/traits.h>
#include <ice/json/value.h>
#include <map>
#include <string>

namespace ice {

// Adds support for 'std::map<V, T>' (de)serialization.
// This is relatively slow. You should write a custom 'json_traits' specialization that is optimized for your std::map key type.
template <typename T, typename V>
struct json_traits<std::map<V, T>> {
  // Used by the 'ice::json::value' constructor and assignment operator.
  static void assign(json::value& self, std::map<V, T> v)
  {
    self.reset(json::type::object);
    for (auto& e : v) {
      self[json::value(e.first).as<std::string>()] = e.second;
    }
  }

  // Used by 'ice::json::value::is()'.
  static bool is(const json::value& self)
  {
    return self.is<json::object>();
  }

  // Used by 'ice::json::value::as()'.
  static std::map<V, T> as(const json::value& self)
  {
    std::map<V, T> v;
    auto type = self.type();
    if (type == json::type::object) {
      for (const auto& e : self) {
        v[json::value(e.name.value()).as<V>()] = e.value.as<T>();
      }
    } else if (type == json::type::array) {
      std::size_t index = 0;
      for (const auto& e : self) {
        v[json::value(index).as<V>()] = e.value.as<T>();
        index++;
      }
    }
    return v;
  }
};

}  // namespace ice