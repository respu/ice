#include <ice/json/parser.h>
#include <ice/json/exception.h>
#include <ice/json/traits.h>
#include <ice/utf8.h>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace ice {
namespace json {
namespace {

parse_error error(const std::string& message, const parser* parser)
{
  if (!parser) {
    return message;
  }
  std::ostringstream oss;
  oss << message << " (";
  if (parser->line() > 0) {
    oss << "line " << parser->line() << " column " << parser->column() << " ";
  }
  oss << parser->current_state() << ")";
  return oss.str();
}

parse_error error(const std::string& message, const parser* parser, char c)
{
  if (!parser)
    return message;
  std::ostringstream oss;
  oss << message << " (";
  if (parser->line() > 0) {
    oss << "line " << parser->line() << " column " << parser->column() << " '";
    if (c >= ' ' || c <= '~') {
      oss << c;
    } else {
      oss << (static_cast<unsigned int>(c) & 0xFF);
    }
    oss << "' ";
  }
  oss << parser->current_state() << ")";
  return oss.str();
}

inline bool is_valid_utf8(std::string::iterator begin, std::string::iterator end)
{
  return utf8::is_valid(begin, end);
}

inline bool utf16to8(std::string& dst, const wchar_t* src, std::size_t size)
{
  try {
    utf8::utf16to8(src, src + size, std::back_inserter<std::string>(dst));
    return true;
  }
  catch (...) {
  }
  return false;
}

inline bool is_space(char c)
{
  switch (c) {
  case ' ':
  case '\t':
  case '\r':
  case '\n':
  case '\v':
  case '\f':
    return true;
  }
  return false;
}

inline bool is_digit(char c)
{
  switch (c) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return true;
  }
  return false;
}

inline wchar_t utf16_hex_digit(char c)
{
  switch (c) {
  case '0': return 0x0;
  case '1': return 0x1;
  case '2': return 0x2;
  case '3': return 0x3;
  case '4': return 0x4;
  case '5': return 0x5;
  case '6': return 0x6;
  case '7': return 0x7;
  case '8': return 0x8;
  case '9': return 0x9;
  case 'a':
  case 'A': return 0xA;
  case 'b':
  case 'B': return 0xB;
  case 'c':
  case 'C': return 0xC;
  case 'd':
  case 'D': return 0xD;
  case 'e':
  case 'E': return 0xE;
  case 'f':
  case 'F': return 0xF;
  }
  return 0xFF;
}

// Returns the number of UTF-16 code units required to represend a unicode code point starting with this code unit.
inline unsigned char utf16_size(wchar_t w)
{
  if (w > 0xD7FF && w < 0xE000) {
    return 2;
  }
  return 1;
}

// Converts the given four characters that represent hexadecimal digits into a single UTF-16 code unit.
inline bool utf16_code(const char* buffer, wchar_t& w)
{
  wchar_t c = utf16_hex_digit(buffer[0]);
  if (c == 0xFF) {
    return false;
  }
  w = c << 12;
  c = utf16_hex_digit(buffer[1]);
  if (c == 0xFF) {
    return false;
  }
  w |= c << 8;
  c = utf16_hex_digit(buffer[2]);
  if (c == 0xFF) {
    return false;
  }
  w |= c << 4;
  c = utf16_hex_digit(buffer[3]);
  if (c == 0xFF) {
    return false;
  }
  w |= c;
  return true;
}

}  // namespace

bool parser::put(char c)
{
  if (c == '\n') {
    line_++;
    column_ = 0;
  }
  column_++;
  switch (state_) {
    // The value is empty. Expecting an array, an object, a native type or a whitespace character.
  case state::start:
    if (is_space(c)) {
      break;
    }
    if (static_cast<unsigned char>(c) == 0xEF) {
      state_ = state::bom_0;
      break;
    }
    if (c == '-' || c == '.' || is_digit(c)) {
      buffer_ = c;
      value_ = 0.0;
      state_ = state::number;
      break;
    }
    switch (c) {
    case '/':
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    case 'n':
      value_ = json::null();
      state_ = state::null_n;
      break;
    case 't':
      value_ = true;
      state_ = state::true_t;
      break;
    case 'f':
      value_ = false;
      state_ = state::false_f;
      break;
    case '"':
      value_ = json::string();
      state_ = state::string;
      break;
    case '[':
      inner_value_ = std::make_unique<parser>();
      inner_value_->line_ = line_;
      inner_value_->column_ = column_;
      value_ = json::array();
      state_ = state::array;
      break;
    case '{':
      inner_value_ = std::make_unique<parser>();
      inner_value_->line_ = line_;
      inner_value_->column_ = column_;
      value_ = json::object();
      state_ = state::object;
      break;
    default:
      throw error("syntax error", this, c);
    }
    break;
    // Received 0xEF from a BOM sequence. Expecting 0xBB.
  case state::bom_0:
    if (static_cast<unsigned char>(c) != 0xBB) {
      throw error("BOM syntax error", this, c);
    }
    state_ = state::bom_1;
    break;
    // Received 0xEF 0xBB from a BOM sequence. Expecting 0xBF.
  case state::bom_1:
    if (static_cast<unsigned char>(c) != 0xBF) {
      throw error("BOM syntax error", this, c);
    }
    state_ = state::start;
    break;
    // Received "/" from "//". Expecting '/'.
  case state::comment_0:
    if (c != '/') {
      throw error("syntax error", this, c);
    }
    state_ = state::comment_1;
    break;
    // Received "/" from "//". Ignoring all characters until we get '\n'.
  case state::comment_1:
    if ((c & 0x80) == 0) {
      if (c == '\n') {
        state_ = comment_state_;
      }
    } else if ((c & 0xE0) == 0xC0) {
      state_ = state::comment_u2;
    } else if ((c & 0xF0) == 0xE0) {
      state_ = state::comment_u3;
    } else if ((c & 0xF8) == 0xF0) {
      state_ = state::comment_u4;
    } else {
      throw error("invalid UTF-8", this, c);
    }
    break;
    // Received a multi-byte UTF-8 character in a comment and still expecting the last byte.
  case state::comment_u2:
    state_ = state::comment_1;
    break;
    // Received a multi-byte UTF-8 character in a comment and still expecting the two byte.
  case state::comment_u3:
    state_ = state::comment_u2;
    break;
    // Received a multi-byte UTF-8 character in a comment and still expecting the three byte.
  case state::comment_u4:
    state_ = state::comment_u3;
    break;
    // Received "n" from "null". Expecting 'u'.
  case state::null_n:
    if (c != 'u') {
      throw error("syntax error", this, c);
    }
    state_ = state::null_u;
    break;
    // Received "nu" from "null". Expecting the 1st 'l'.
  case state::null_u:
    if (c != 'l') {
      throw error("syntax error", this, c);
    }
    state_ = state::null_l;
    break;
    // Received "nul" from "null". Expecting the 2nd 'l'.
  case state::null_l:
    if (c != 'l') {
      throw error("syntax error", this, c);
    }
    state_ = state::end;
    break;
    // Received "t" from "true". Expecting 'r'.
  case state::true_t:
    if (c != 'r') {
      throw error("syntax error", this, c);
    }
    state_ = state::true_r;
    break;
    // Received "tr" from "true". Expecting 'u'.
  case state::true_r:
    if (c != 'u') {
      throw error("syntax error", this, c);
    }
    state_ = state::true_u;
    break;
    // Received "tre" from "true". Expecting 'e'.
  case state::true_u:
    if (c != 'e') {
      throw error("syntax error", this, c);
    }
    state_ = state::end;
    break;
    // Received "f" from "false". Expecting 'a'.
  case state::false_f:
    if (c != 'a') {
      throw error("syntax error", this, c);
    }
    state_ = state::false_a;
    break;
    // Received "fa" from "false". Expecting 'l'.
  case state::false_a:
    if (c != 'l') {
      throw error("syntax error", this, c);
    }
    state_ = state::false_l;
    break;
    // Received "fal" from "false". Expecting 's'.
  case state::false_l:
    if (c != 's') {
      throw error("syntax error", this, c);
    }
    state_ = state::false_s;
    break;
    // Received "fals" from "false". Expecting 'e'.
  case state::false_s:
    if (c != 'e') {
      throw error("syntax error", this, c);
    }
    state_ = state::end;
    break;
    // Received "-", "." or a digit from a number. Expecting non-whitecharacters that we store and process on get().
  case state::number:
  {
    if (is_space(c)) {
      state_ = state::end;
    } else {
      buffer_ += c;
    }
  } break;
  // Received the opening '"' and possibly more characters from a string.
  case state::string:
  {
    if ((c & 0x80) == 0) {
      switch (c) {
      case '"': state_ = state::end; break;
      case '\\': state_ = state::string_escape; break;
      default: value_.data<json::string>() += c; break;
      }
    } else if ((c & 0xE0) == 0xC0) {
      buffer_ = c;
      state_ = state::string_u2;
    } else if ((c & 0xF0) == 0xE0) {
      buffer_ = c;
      state_ = state::string_u3;
    } else if ((c & 0xF8) == 0xF0) {
      buffer_ = c;
      state_ = state::string_u4;
    } else {
      throw error("invalid UTF-8", this, c);
    }
  } break;
  // Received a multi-byte UTF-8 character in a string and still expecting the last byte.
  case state::string_u2:
    buffer_ += c;
    if (!is_valid_utf8(buffer_.begin(), buffer_.end())) {
      throw error("invalid UTF-8", this, c);
    }
    value_.data<json::string>() += buffer_;
    buffer_.clear();
    state_ = state::string;
    break;
    // Received a multi-byte UTF-8 character in a string and still expecting the two byte.
  case state::string_u3:
    buffer_ += c;
    state_ = state::string_u2;
    break;
    // Received a multi-byte UTF-8 character in a string and still expecting the three byte.
  case state::string_u4:
    buffer_ += c;
    state_ = state::string_u3;
    break;
    // Received a '\\' in a string. Expecting a valid escape sequence.
  case state::string_escape:
  {
    switch (c) {
    case '"':
      value_.data<json::string>() += '"';
      state_ = state::string;
      break;
    case '\\':
      value_.data<json::string>() += '\\';
      state_ = state::string;
      break;
    case '/':
      value_.data<json::string>() += '/';
      state_ = state::string;
      break;
    case 'b':
      value_.data<json::string>() += '\b';
      state_ = state::string;
      break;
    case 'f':
      value_.data<json::string>() += '\f';
      state_ = state::string;
      break;
    case 'n':
      value_.data<json::string>() += '\n';
      state_ = state::string;
      break;
    case 'r':
      value_.data<json::string>() += '\r';
      state_ = state::string;
      break;
    case 't':
      value_.data<json::string>() += '\t';
      state_ = state::string;
      break;
    case 'u': state_ = state::string_escape_u_0; break;
    default: throw error("syntax error", this, c); break;
    }
  } break;
  // Received "\\u" in a string. Expecting a hexadecimal character.
  case state::string_escape_u_0:
    buffer_ = c;
    state_ = state::string_escape_u_1;
    break;
    // Received "\\uX" in a string. Expecting a hexadecimal character.
  case state::string_escape_u_1:
    buffer_ += c;
    state_ = state::string_escape_u_2;
    break;
    // Received "\\uXX" in a string. Expecting a hexadecimal character.
  case state::string_escape_u_2:
    buffer_ += c;
    state_ = state::string_escape_u_3;
    break;
    // Received "\\uXXX" in a string. Expecting a hexadecimal character.
  case state::string_escape_u_3:
  {
    buffer_ += c;
    wchar_t w[1];
    if (!utf16_code(&buffer_[0], w[0])) {
      throw error("invalid UTF-16 escape sequence", this, c);
    }
    if (utf16_size(w[0]) == 1) {
      buffer_.clear();
      if (!utf16to8(value_.data<std::string>(), w, 1)) {
        throw error("invalid UTF-16", this, c);
      }
      state_ = state::string;
    } else {
      state_ = state::string_escape_u_4;
    }
  } break;
  // Received a UTF-16 surrogate code unit in a string. Expecting '\\' of the last escape sequence.
  case state::string_escape_u_4:
    if (c != '\\') {
      throw error("invalid UTF-16 escape sequence", this, c);
    }
    state_ = state::string_escape_u_5;
    break;
    // Received a UTF-16 surrogate code unit in a string. Expecting 'u' of the last escape sequence.
  case state::string_escape_u_5:
    if (c != 'u') {
      throw error("invalid UTF-16 escape sequence", this, c);
    }
    state_ = state::string_escape_u_6;
    break;
    // Received a UTF-16 surrogate code unit in a string. Expecting the 1st hexadecimal digit of the last escape sequence.
  case state::string_escape_u_6:
    buffer_ += c;
    state_ = state::string_escape_u_7;
    break;
    // Received a UTF-16 surrogate code unit in a string. Expecting the 2nd hexadecimal digit of the last escape sequence.
  case state::string_escape_u_7:
    buffer_ += c;
    state_ = state::string_escape_u_8;
    break;
    // Received a UTF-16 surrogate code unit in a string. Expecting the 3rd hexadecimal digit of the last escape sequence.
  case state::string_escape_u_8:
    buffer_ += c;
    state_ = state::string_escape_u_9;
    break;
    // Received a UTF-16 surrogate code unit in a string. Expecting the 4th hexadecimal digit of the last escape sequence.
  case state::string_escape_u_9:
  {
    buffer_ += c;
    wchar_t w[2];
    if (!utf16_code(&buffer_[0], w[0])) {
      throw error("invalid UTF-16 escape sequence", this, c);
    }
    if (!utf16_code(&buffer_[4], w[1])) {
      throw error("invalid UTF-16 escape sequence", this, c);
    }
    buffer_.clear();
    if (!utf16to8(value_.data<std::string>(), w, 2)) {
      throw error("invalid UTF-16", this, c);
    }
    state_ = state::string;
  } break;
  // Received "[". Redirecting input to inner value. Expecting ',' or ']' to stop redirecting.
  case state::array:
  {
    auto inner_value_state = inner_value_->state_;
    if (c == '/' &&
      (inner_value_state == state::start || inner_value_state == state::number || inner_value_state == state::end)) {
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    }
    if (c == ',' && inner_value_state == state::number) {
      append_inner_value();
      state_ = state::array;
      break;
    }
    if (c == ']') {
      if (inner_value_state == state::start) {
        state_ = state::end;
        break;
      }
      if (inner_value_state == state::number || inner_value_state == state::end) {
        append_inner_value();
        state_ = state::end;
        break;
      }
    }
    if (inner_value_->put(c)) {
      append_inner_value();
      state_ = state::array_value;
      break;
    }
  } break;
  // Received and appended an array value. Expecting ',' to continue redirecting or ']' to stop redirecting.
  case state::array_value:
  {
    if (c == '/') {
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    }
    if (c == ',') {
      state_ = state::array;
      break;
    }
    if (c == ']') {
      state_ = state::end;
      break;
    }
    if (!is_space(c)) {
      throw error("invalid array syntax", this, c);
    }
  } break;
  // Received "{". Expecting '"' or '}'.
  case state::object:
  {
    if (c == '/') {
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    }
    if (c == '}') {
      state_ = state::end;
      break;
    }
    if (c == '"') {
      inner_value_->clear();
      inner_value_->put(c);
      state_ = state::object_start;
      break;
    }
    if (!is_space(c)) {
      throw error("invalid object syntax", this, c);
    }
  } break;
  // Received "{ \"". Redirecting input to inner value. Expecting the name to be complete to stop redirecting.
  case state::object_start:
    if (inner_value_->put(c)) {
      buffer_ = inner_value_->get().data<json::string>();
      state_ = state::object_name;
    }
    break;
    // Received "{ \"...\"" with the name stored in buffer_. Expecting ':' or a whitespace.
  case state::object_name:
  {
    if (c == '/') {
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    }
    if (c == ':') {
      inner_value_->clear();
      state_ = state::object_semicolon;
      break;
    }
    if (!is_space(c)) {
      throw error("invalid object syntax", this, c);
    }
  } break;
  // Received "{ \"...\" : " with the name stored in buffer_. Redirecting input to inner value.
  case state::object_semicolon:
  {
    auto inner_value_state = inner_value_->state_;
    if (c == '/' &&
      (inner_value_state == state::start || inner_value_state == state::number || inner_value_state == state::end)) {
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    }
    if (c == ',' && inner_value_state == state::number) {
      assign_inner_value();
      state_ = state::object;
      break;
    }
    if (c == '}' && (inner_value_state == state::number || inner_value_state == state::end)) {
      assign_inner_value();
      state_ = state::end;
      break;
    }
    if (inner_value_->put(c)) {
      assign_inner_value();
      state_ = state::object_value;
      break;
    }
  } break;
  // Received and assigned an object value. Expecting ',' to continue redirecting or '}' to stop redirecting.
  case state::object_value:
  {
    if (c == '/') {
      comment_state_ = state_;
      state_ = state::comment_0;
      break;
    }
    if (c == ',') {
      state_ = state::object;
      break;
    }
    if (c == '}') {
      state_ = state::end;
      break;
    }
    if (!is_space(c)) {
      throw error("invalid object syntax", this, c);
    }
  } break;
  case state::end: return true;
  }
  return state_ == state::end;
}

bool parser::complete() const
{
  return state_ == state::end || value_.type() == type::number;
}

value& parser::get()
{
  if (value_.type() == type::number) {
    std::istringstream iss(buffer_);
    double value = 0.0;
    //iss >> std::setprecision(std::numeric_limits<double>::max_digits10) >> value;
    iss >> value;
    value_ = value;
    state_ = state::start;
    buffer_.clear();
    return value_;
  }
  if (state_ != state::end) {
    throw error("incomplete input", this);
  }
  return value_;
}

void parser::clear()
{
  buffer_.clear();
  value_.clear();
  state_ = state::start;
}

std::size_t parser::line() const
{
  return line_;
}

std::size_t parser::column() const
{
  return column_;
}

parser::state parser::current_state() const
{
  return state_;
}

void parser::append_inner_value()
{
  value_.append(std::move(inner_value_->get()));
  inner_value_->clear();
}

void parser::assign_inner_value()
{
  value_[buffer_] = std::move(inner_value_->get());
  inner_value_->clear();
}

std::ostream& operator<<(std::ostream& os, parser::state state)
{
  switch (state) {
  case parser::state::start: os << "parsing a new JSON value"; break;
  case parser::state::bom_0: os << "parsing 0xBB from a BOM sequence"; break;
  case parser::state::bom_1: os << "parsing 0xBF from a BOM sequence"; break;
  case parser::state::comment_0: os << "parsing the 2nd '/' from a comment"; break;
  case parser::state::comment_1: os << "parsing '\n' to end the comment"; break;
  case parser::state::comment_u2: os << "parsing the last byte of a multibyte unicode character in a comment"; break;
  case parser::state::comment_u3: os << "parsing the last two bytes of a multibyte unicode character in a comment"; break;
  case parser::state::comment_u4: os << "parsing the last three bytes of a multibyte unicode character in a comment"; break;
  case parser::state::null_n: os << "parsing 'u' from 'null'"; break;
  case parser::state::null_u: os << "parsing the 1st 'l' from 'null'"; break;
  case parser::state::null_l: os << "parsing the 2nd 'l' from 'null'"; break;
  case parser::state::true_t: os << "parsing 'r' from 'true'"; break;
  case parser::state::true_r: os << "parsing 'u' from 'true'"; break;
  case parser::state::true_u: os << "parsing 'e' from 'true'"; break;
  case parser::state::false_f: os << "parsing 'a' from 'false'"; break;
  case parser::state::false_a: os << "parsing 'l' from 'false'"; break;
  case parser::state::false_l: os << "parsing 's' from 'false'"; break;
  case parser::state::false_s: os << "parsing 'e' from 'false'"; break;
  case parser::state::number: os << "parsing a number"; break;
  case parser::state::string: os << "parsing a string"; break;
  case parser::state::string_u2: os << "parsing the last byte of a multibyte unicode character in a string"; break;
  case parser::state::string_u3: os << "parsing the last two bytes of a multibyte unicode character in a string"; break;
  case parser::state::string_u4: os << "parsing the last three bytes of a multibyte unicode character in a string"; break;
  case parser::state::string_escape: os << "parsing a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_0: os << "parsing the 1st digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_1: os << "parsing the 2nd digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_2: os << "parsing the 3rd digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_3: os << "parsing the 4th digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_4: os << "parsing the 5th digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_5: os << "parsing the 6th digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_6: os << "parsing the 7th digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_7: os << "parsing the 8th digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_8: os << "parsing the 9th digit from a UTF-16 escape sequence"; break;
  case parser::state::string_escape_u_9: os << "parsing the 10th digit from a UTF-16 escape sequence"; break;
  case parser::state::array: os << "parsing an array value"; break;
  case parser::state::array_value: os << "parsing ',' or ']' from an array"; break;
  case parser::state::object: os << "parsing '\"' or '}' from an object"; break;
  case parser::state::object_start: os << "parsing '\"' from an object key"; break;
  case parser::state::object_name: os << "parsing ':' from an object key"; break;
  case parser::state::object_semicolon: os << "parsing the object value"; break;
  case parser::state::object_value: os << "parsing ',' or '}' from an object"; break;
  case parser::state::end: os << "finished parsing";
  default: os << "(unknown)"; break;
  }
  return os;
}

}  // namespace json
}  // namespace ice