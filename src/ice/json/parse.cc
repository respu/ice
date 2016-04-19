#include <ice/json/parse.h>
#include <ice/json/parser.h>
#include <sstream>

namespace ice {
namespace json {

value parse(std::istream& is)
{
  json::parser parser;
  char c = '\0';
  while (is.get(c)) {
    if (parser.put(c)) {
      return parser.get();
    }
  }
  return parser.get();
}
value parse(const std::string& text)
{
  json::parser parser;
  for (auto c : text) {
    if (parser.put(c)) {
      return parser.get();
    }
  }
  return parser.get();
}

value parse(const char* text, std::size_t size)
{
  if (!text)
    return value();
  json::parser parser;
  for (std::size_t i = 0; i < size; i++) {
    if (parser.put(text[i])) {
      return parser.get();
    }
  }
  return parser.get();
}

}  // namespace json
}  // namespace ice