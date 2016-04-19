#include <ice/json/types.h>
#include <ice/json/exception.h>
#include <ice/json/value.h>
#include <sstream>

namespace ice {
namespace json {

std::ostream& operator<<(std::ostream& os, json::type type)
{
  switch (type) {
  case json::type::null: return os << "null";
  case json::type::boolean: return os << "boolean";
  case json::type::number: return os << "number";
  case json::type::string: return os << "string";
  case json::type::array: return os << "array";
  case json::type::object: return os << "object";
  }
  return os << "unknown (" << static_cast<int>(type) << ")";
}

}  // namespace json
}  // namespace ice