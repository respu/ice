#pragma once
#include <ice/json/value.h>
#include <istream>
#include <string>

namespace ice {
namespace json {

value parse(std::istream& is);
value parse(const std::string& text);
value parse(const char* src, std::size_t size);

}  // namespace json
}  // namespace ice