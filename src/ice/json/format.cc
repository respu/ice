#include <ice/json/format.h>
#include <ice/utf8.h>
#include <iomanip>
#include <limits>

namespace ice {
namespace json {
namespace {

inline char utf16_hex_char(unsigned char c)
{
  switch (c & 0x0F) {
  case 0x0: return '0';
  case 0x1: return '1';
  case 0x2: return '2';
  case 0x3: return '3';
  case 0x4: return '4';
  case 0x5: return '5';
  case 0x6: return '6';
  case 0x7: return '7';
  case 0x8: return '8';
  case 0x9: return '9';
  case 0xA: return 'a';
  case 0xB: return 'b';
  case 0xC: return 'c';
  case 0xD: return 'd';
  case 0xE: return 'e';
  case 0xF: return 'f';
  }
  return '0';
}

inline std::ostream& format(std::ostream& os, const string& v)
{
  for (auto it = v.begin(); it != v.end(); ++it) {
    char c = *it;
    if ((c & 0x80) == 0) {
      switch (c) {
      case '"': os << "\\\""; break;
      case '\\': os << "\\\\"; break;
      case '\b': os << "\\b"; break;
      case '\f': os << "\\f"; break;
      case '\n': os << "\\n"; break;
      case '\r': os << "\\r"; break;
      case '\t': os << "\\t"; break;
      default:
        if (c < 0x20 || c == 0x7F || (static_cast<unsigned char>(c) >= 0x80 && static_cast<unsigned char>(c) <= 0x9F)) {
          os << "\\u00" << utf16_hex_char(static_cast<unsigned char>(c) >> 4)
            << utf16_hex_char(static_cast<unsigned char>(c));
        } else {
          os << c;
        }
        break;
      }
    } else if ((c & 0xE0) == 0xC0) {
      ++it;
      if (it == v.end() || ((*it) & 0xC0) != 0x80) {
        throw std::runtime_error("invalid UTF-8");
      }
      os << c << *it;
    } else if ((c & 0xF0) == 0xE0) {
      ++it;
      if (it == v.end() || ((*it) & 0xC0) != 0x80) {
        throw std::runtime_error("invalid UTF-8");
      }
      char c2 = *it;
      ++it;
      if (it == v.end() || ((*it) & 0xC0) != 0x80) {
        throw std::runtime_error("invalid UTF-8");
      }
      os << c << c2 << *it;
    } else if ((c & 0xF8) == 0xF0) {
      ++it;
      if (it == v.end() || ((*it) & 0xC0) != 0x80) {
        throw std::runtime_error("invalid UTF-8");
      }
      char c2 = *it;
      ++it;
      if (it == v.end() || ((*it) & 0xC0) != 0x80) {
        throw std::runtime_error("invalid UTF-8");
      }
      char c3 = *it;
      ++it;
      if (it == v.end() || ((*it) & 0xC0) != 0x80) {
        throw std::runtime_error("invalid UTF-8");
      }
      os << c << c2 << c3 << *it;
    } else {
      throw std::runtime_error("invalid UTF-8");
    }
  }
  return os;
}

inline std::ostream& format(std::ostream& os, const array& v, bool pretty, std::size_t offset)
{
  if (v.empty()) {
    return os << "[]";
  }
  os << '[';
  for (auto it = v.begin(); it != v.end(); ++it) {
    if (it != v.begin()) {
      os << ',';
    }
    if (pretty) {
      os << '\n' << std::string(offset + 2, ' ');
    }
    format(os, it->value, pretty, offset + 2);
  }
  if (pretty) {
    os << '\n' << std::string(offset, ' ');
  }
  return os << ']';
}

inline std::ostream& format(std::ostream& os, const object& v, bool pretty, std::size_t offset)
{
  if (v.empty()) {
    return os << "{}";
  }
  os << '{';
  for (auto it = v.begin(); it != v.end(); ++it) {
    if (it != v.begin()) {
      os << ',';
    }
    if (pretty) {
      os << '\n' << std::string(offset + 2, ' ');
    }
    os << '"';
    format(os, it->name.value());
    os << "\":";
    if (pretty) {
      os << ' ';
    }
    format(os, it->value, pretty, offset + 2);
  }
  if (pretty) {
    os << '\n' << std::string(offset, ' ');
  }
  return os << '}';
}

}  // namespace

std::string format(const value& root, bool pretty)
{
  std::ostringstream oss;
  format(oss, root, pretty);
  return oss.str();
}

std::ostream& format(std::ostream& os, const json::value& v, bool pretty, std::size_t offset)
{
  switch (v.type_) {
  case type::null: return os << "null";
  case type::boolean: return os << (v.data_.boolean ? "true" : "false");
  case type::number: return os << v.data_.number;
  case type::string:
    os << '"';
    format(os, v.data_.string);
    return os << '"';
  case type::array: return format(os, v.data_.array, pretty, offset);
  case type::object: return format(os, v.data_.object, pretty, offset);
  }
  return os;
}

}  // namespace json
}  // namespace ice