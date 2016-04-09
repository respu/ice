#pragma once
#include <ice/string_view.h>
#include <string>
#include <system_error>

namespace ice {

// Converts an UTF-8 string to a UTF-16 string. Throws on error.
std::string convert(const ice::u16string_view src);

// Converts an UTF-8 string to a UTF-16 string. Stops on error.
std::string convert(const ice::u16string_view src, std::error_code& ec);

// Converts an UTF-16 string to a UTF-8 string. Throws on error.
std::u16string convert(const ice::string_view src);

// Converts an UTF-16 string to a UTF-8 string. Stops on error.
std::u16string convert(const ice::string_view src, std::error_code& ec);

// Returns true when the given string view contains a valid UTF-8 sequence.
bool is_valid_utf8(ice::string_view str);

}  // namespace ice