#pragma once
#include <ice/json/value.h>
#include <ostream>
#include <cstdint>

namespace ice {
namespace json {

// Serializes the given json value.
// When 'pretty' is true, additional spacing is added.
std::string format(const value& root, bool pretty = true);

// Serializes the given json value.
// When 'pretty' is true, additional spacing is added. 
std::ostream& format(std::ostream& os, const value& root, bool pretty = true, std::size_t offset = 0);

}  // namespace json
}  // namespace ice