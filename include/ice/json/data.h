#pragma once
#include <ice/json/types.h>

namespace ice {
namespace json {

union data {
  json::null null;
  json::boolean boolean;
  json::number number;
  json::string string;
  json::array array;
  json::object object;

  data()
  {}

  ~data()
  {}
};

}  // namespace json
}  // namespace ice