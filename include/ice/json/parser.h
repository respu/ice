#pragma once
#include <ice/json/value.h>
#include <memory>
#include <cstdint>

namespace ice {
namespace json {

class parser {
public:
  enum struct state {
    start = 0,
    bom_0,
    bom_1,
    comment_0,
    comment_1,
    comment_u2,
    comment_u3,
    comment_u4,
    null_n,
    null_u,
    null_l,
    true_t,
    true_r,
    true_u,
    false_f,
    false_a,
    false_l,
    false_s,
    number,
    string,
    string_u2,
    string_u3,
    string_u4,
    string_escape,
    string_escape_u_0,
    string_escape_u_1,
    string_escape_u_2,
    string_escape_u_3,
    string_escape_u_4,
    string_escape_u_5,
    string_escape_u_6,
    string_escape_u_7,
    string_escape_u_8,
    string_escape_u_9,
    array,
    array_value,
    object,
    object_start,
    object_name,
    object_semicolon,
    object_value,
    end
  };

  bool put(char c);
  bool complete() const;

  value& get();

  void clear();

  std::size_t line() const;
  std::size_t column() const;
  state current_state() const;

private:
  void append_inner_value();
  void assign_inner_value();

  value value_;
  std::unique_ptr<parser> inner_value_;
  state state_ = state::start;
  state comment_state_ = state::start;
  std::string buffer_;
  std::size_t line_ = 1;
  std::size_t column_ = 0;
};

std::ostream& operator<<(std::ostream& os, parser::state state);

}  // namespace json
}  // namespace ice