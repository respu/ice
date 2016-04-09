#pragma once
#include <system_error>

namespace ice {
namespace windows {

const std::error_category& error_category();

std::error_code make_error();
std::error_code make_error(unsigned error);

}  // namespace windows
}  // namespace ice