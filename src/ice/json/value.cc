#include <ice/json/format.h>
#include <ice/json/traits.h>
#include <ice/json/value.h>
#include <algorithm>
#include <sstream>

namespace ice {
namespace json {

value::value(value&& other) : type_(other.type_)
{
  switch (type_) {
  case json::type::null: break;
  case json::type::boolean: data_.boolean = other.data_.boolean; break;
  case json::type::number: data_.number = other.data_.number; break;
  case json::type::string: new (&data_.string) string(std::move(other.data_.string)); break;
  case json::type::array: new (&data_.array) array(std::move(other.data_.array)); break;
  case json::type::object: new (&data_.object) object(std::move(other.data_.object)); break;
  }
}

value::value(const value& other) : type_(other.type_)
{
  switch (type_) {
  case json::type::null: break;
  case json::type::boolean: data_.boolean = other.data_.boolean; break;
  case json::type::number: data_.number = other.data_.number; break;
  case json::type::string: new (&data_.string) string(other.data_.string); break;
  case json::type::array: new (&data_.array) array(other.data_.array); break;
  case json::type::object: new (&data_.object) object(other.data_.object); break;
  }
}

value::value(std::initializer_list<value> list)
{
  bool is_object = true;
  for (auto& e : list) {
    if (e.size() != 2) {
      is_object = false;
      break;
    }
  }
  if (is_object) {
    for (auto& e : list) {
      operator[](e[0].as_string()) = e[1];
    }
  } else {
    for (auto& e : list) {
      append(e);
    }
  }
}

value::~value()
{
  reset();
}

value& value::operator=(value&& other)
{
  switch (other.type_) {
  case json::type::null: reset(); break;
  case json::type::boolean: reset(other.data_.boolean); break;
  case json::type::number: reset(other.data_.number); break;
  case json::type::string: reset(std::move(other.data_.string)); break;
  case json::type::array: reset(std::move(other.data_.array)); break;
  case json::type::object: reset(std::move(other.data_.object)); break;
  }
  return *this;
}

value& value::operator=(const value& other)
{
  switch (other.type_) {
  case json::type::null: reset(); break;
  case json::type::boolean: reset(other.data_.boolean); break;
  case json::type::number: reset(other.data_.number); break;
  case json::type::string: reset(other.data_.string); break;
  case json::type::array: reset(other.data_.array); break;
  case json::type::object: reset(other.data_.object); break;
  }
  return *this;
}

json::type value::type() const
{
  return type_;
}

bool value::empty() const noexcept
{
  switch (type_) {
  case json::type::array: return data_.array.empty();
  case json::type::object: return data_.object.empty();
  default: return true;
  }
}

std::size_t value::size() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.size();
    case json::type::object: return data_.object.size();
    default: return 0;
    }
  }
  catch (...) {
    return 0;
  }
}

void value::clear() noexcept
{
  try {
    switch (type_) {
    case json::type::string: data_.string.clear(); break;
    case json::type::array: data_.array.clear(); break;
    case json::type::object: data_.object.clear(); break;
    default: break;
    }
  }
  catch (...) {
  }
}

value& value::operator[](std::size_t index)
{
  if (type_ != json::type::array) {
    throw type_error::access(type_, index);
  }
  if (data_.array.size() <= index) {
    throw range_error::access(index);
  }
  return data_.array[index].value;
}

const value& value::operator[](std::size_t index) const
{
  if (type_ != json::type::array) {
    throw type_error::const_access(type_, index);
  }
  if (data_.array.size() <= index) {
    throw range_error::const_access(index);
  }
  return data_.array[index].value;
}

void value::erase(std::size_t index)
{
  if (type_ != json::type::array) {
    throw type_error::erase(type_, index);
  }
  if (data_.array.size() <= index) {
    throw range_error::access(index);
  }
  data_.array.erase(data_.array.begin() + index);
}

value& value::operator[](const std::string& key)
{
  if (type_ != json::type::object) {
    reset(json::type::object);
  }
  auto it = std::find_if(data_.object.begin(), data_.object.end(), [&key](auto& e) {
    return e.name && e.name.value() == key;
  });
  if (it == data_.object.end()) {
    return data_.object.insert(data_.object.end(), { key, ice::json::value() })->value;
  }
  return it->value;
}

const value& value::operator[](const std::string& key) const
{
  if (type_ != json::type::object) {
    throw type_error::const_access(type_, key);
  }
  auto it = std::find_if(data_.object.cbegin(), data_.object.cend(), [&key](auto& e) {
    return e.name && e.name.value() == key;
  });
  if (it == data_.object.end()) {
    throw range_error::const_access(key);
  }
  return it->value;
}

iterator value::find(const std::string& key) noexcept
{
  try {
    if (type_ == json::type::object) {
      auto it = std::find_if(data_.object.begin(), data_.object.end(), [&key](auto& e) {
        return e.name && e.name.value() == key;
      });
      if (it != data_.object.end()) {
        return it;
      }
    }
  }
  catch (...) {
  }
  return iterator();
}

const_iterator value::find(const std::string& key) const noexcept
{
  try {
    if (type_ == json::type::object) {
      auto it = std::find_if(data_.object.cbegin(), data_.object.cend(), [&key](auto& e) {
        return e.name && e.name.value() == key;
      });
      if (it != data_.object.end()) {
        return it;
      }
    }
  }
  catch (...) {
  }
  return iterator();
}

iterator value::erase(const std::string& key) noexcept
{
  try {
    if (type_ == json::type::object) {
      auto it = std::find_if(data_.object.cbegin(), data_.object.cend(), [&key](auto& e) {
        return e.name && e.name.value() == key;
      });
      if (it != data_.object.end()) {
        return data_.object.erase(it);
      }
    }
  }
  catch (...) {
  }
  return iterator();
}

iterator value::begin() noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.begin();
    case json::type::object: return data_.object.begin();
    default: return iterator();
    }
  }
  catch (...) {
  }
  return iterator();
}

const_iterator value::begin() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.begin();
    case json::type::object: return data_.object.begin();
    default: return const_iterator();
    }
  }
  catch (...) {
  }
  return const_iterator();
}

const_iterator value::cbegin() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.cbegin();
    case json::type::object: return data_.object.cbegin();
    default: return const_iterator();
    }
  }
  catch (...) {
  }
  return const_iterator();
}

iterator value::end() noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.end();
    case json::type::object: return data_.object.end();
    default: return iterator();
    }
  }
  catch (...) {
  }
  return iterator();
}

const_iterator value::end() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.end();
    case json::type::object: return data_.object.end();
    default: return const_iterator();
    }
  }
  catch (...) {
  }
  return const_iterator();
}

const_iterator value::cend() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.cend();
    case json::type::object: return data_.object.cend();
    default: return const_iterator();
    }
  }
  catch (...) {
  }
  return const_iterator();
}

reverse_iterator value::rbegin() noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.rbegin();
    case json::type::object: return data_.object.rbegin();
    default: return reverse_iterator();
    }
  }
  catch (...) {
  }
  return reverse_iterator();
}

const_reverse_iterator value::rbegin() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.rbegin();
    case json::type::object: return data_.object.rbegin();
    default: return const_reverse_iterator();
    }
  }
  catch (...) {
  }
  return const_reverse_iterator();
}

const_reverse_iterator value::crbegin() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.crbegin();
    case json::type::object: return data_.object.crbegin();
    default: return const_reverse_iterator();
    }
  }
  catch (...) {
  }
  return const_reverse_iterator();
}

reverse_iterator value::rend() noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.rend();
    case json::type::object: return data_.object.rend();
    default: return reverse_iterator();
    }
  }
  catch (...) {
  }
  return reverse_iterator();
}

const_reverse_iterator value::rend() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.rend();
    case json::type::object: return data_.object.rend();
    default: return const_reverse_iterator();
    }
  }
  catch (...) {
  }
  return const_reverse_iterator();
}

const_reverse_iterator value::crend() const noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.crend();
    case json::type::object: return data_.object.crend();
    default: return const_reverse_iterator();
    }
  }
  catch (...) {
  }
  return const_reverse_iterator();
}

iterator value::erase(iterator it)  noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.erase(it); break;
    case json::type::object: return data_.object.erase(it); break;
    default: return iterator();
    }
  }
  catch (...) {
  }
  return iterator();
}

iterator value::erase(const_iterator it) noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.erase(it); break;
    case json::type::object: return data_.object.erase(it); break;
    default: return iterator();
    }
  }
  catch (...) {
  }
  return iterator();
}

iterator value::erase(iterator first, iterator last) noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.erase(first, last); break;
    case json::type::object: return data_.object.erase(first, last); break;
    default: return iterator();
    }
  }
  catch (...) {
  }
  return iterator();
}

iterator value::erase(const_iterator first, const_iterator last) noexcept
{
  try {
    switch (type_) {
    case json::type::array: return data_.array.erase(first, last); break;
    case json::type::object: return data_.object.erase(first, last); break;
    default: return iterator();
    }
  }
  catch (...) {
  }
  return iterator();
}

void value::reset()
{
  switch (type_) {
  case json::type::null: return;
  case json::type::string: data_.string.~string(); break;
  case json::type::array: data_.array.~array(); break;
  case json::type::object: data_.object.~object(); break;
  default: break;
  }
  type_ = json::type::null;
}

void value::reset(json::type type)
{
  if (type_ == type) {
    switch (type_) {
    case json::type::string: data_.string.clear(); return;
    case json::type::array: data_.array.clear(); return;
    case json::type::object: data_.object.clear(); return;
    default: break;
    }
  } else {
    reset();
  }
  switch (type) {
  case json::type::null: break;
  case json::type::boolean: data_.boolean = false; break;
  case json::type::number: data_.number = 0.0; break;
  case json::type::string: new (&data_.string) string(); break;
  case json::type::array: new (&data_.array) array(); break;
  case json::type::object: new (&data_.object) object(); break;
  }
  type_ = type;
}

void value::reset(boolean v)
{
  reset();
  data_.boolean = v;
  type_ = json::type::boolean;
}

void value::reset(number v)
{
  reset();
  data_.number = v;
  type_ = json::type::number;
}

void value::reset(string v)
{
  if (type_ == json::type::string) {
    data_.string = std::move(v);
  } else {
    reset();
    new (&data_.string) string(std::move(v));
    type_ = json::type::string;
  }
}

void value::reset(array v)
{
  if (type_ == json::type::array) {
    data_.array = std::move(v);
  } else {
    reset();
    new (&data_.array) array(std::move(v));
    type_ = json::type::array;
  }
}

void value::reset(object v)
{
  if (type_ == json::type::object) {
    data_.object = std::move(v);
  } else {
    reset();
    new (&data_.object) object(std::move(v));
    type_ = json::type::object;
  }
}

boolean value::as_boolean() const noexcept
{
  try {
    switch (type_) {
    case json::type::null: return false;
    case json::type::number: return data_.number != 0.0 ? true : false;
    case json::type::boolean: return data_.boolean;
    case json::type::string: return data_.string == "true" ? true : false;
    case json::type::array: return !empty();
    case json::type::object: return !empty();
    }
  }
  catch (...) {
  }
  return false;
}

number value::as_number() const noexcept
{
  try {
    switch (type_) {
    case json::type::null: return 0;
    case json::type::number: return data_.number;
    case json::type::boolean: return data_.boolean ? 1 : 0;
    case json::type::string:
    {
      std::istringstream iss(data_.string);
      number v = 0;
      iss >> v;
      return v;
    } break;
    case json::type::array: return static_cast<number>(size());
    case json::type::object: return static_cast<number>(size());
    }
  }
  catch (...) {
  }
  return 0;
}

string value::as_string() const noexcept
{
  try {
    switch (type_) {
    case json::type::null: return "null";
    case json::type::boolean: return data_.boolean ? "true" : "false";
    case json::type::number:
    {
      std::ostringstream oss;
      oss << data_.number;
      return oss.str();
    } break;
    case json::type::string: return data_.string;
    case json::type::array: return format(*this, false);
    case json::type::object: return format(*this, false);
    }
    return std::string();
  }
  catch (...) {
  }
  return std::string();
}

bool operator==(const value& a, const value& b)
{
  if (a.type() != b.type()) {
    return false;
  }
  switch (a.type()) {
  case json::type::null: return true;
  case json::type::boolean: return a.data<boolean>() == b.data<boolean>();
  case json::type::number: return a.data<number>() == b.data<number>();
  case json::type::string: return a.data<string>() == b.data<string>();
  case json::type::array: return a.data<array>() == b.data<array>();
  case json::type::object: return a.data<object>() == b.data<object>();
  }
  return false;
}
bool operator!=(const value& a, const value& b)
{
  return !(a == b);
}

bool operator<(const value& a, const value& b)
{
  if (a.type() != b.type()) {
    return false;
  }
  switch (a.type()) {
  case json::type::null: return false;
  case json::type::boolean: return a.data<boolean>() < b.data<boolean>();
  case json::type::number: return a.data<number>() < b.data<number>();
  case json::type::string: return a.data<string>() < b.data<string>();
  case json::type::array: return a.data<array>() < b.data<array>();
  case json::type::object: return a.data<object>() < b.data<object>();
  }
  return false;
}

bool operator>(const value& a, const value& b)
{
  if (a.type() != b.type()) {
    return false;
  }
  switch (a.type()) {
  case json::type::null: return false;
  case json::type::boolean: return a.data<boolean>() > b.data<boolean>();
  case json::type::number: return a.data<number>() > b.data<number>();
  case json::type::string: return a.data<string>() > b.data<string>();
  case json::type::array: return a.data<array>() > b.data<array>();
  case json::type::object: return a.data<object>() > b.data<object>();
  }
  return false;
}

bool operator<=(const value& a, const value& b)
{
  return a == b || a < b;
}

bool operator>=(const value& a, const value& b)
{
  return a == b || a > b;
}

std::ostream& operator<<(std::ostream& os, const value& root)
{
  return format(os, root, false);
}

}  // namespace json
}  // namespace ice