#pragma once
#include <ice/exception.h>
#include <ice/optional.h>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace ice {

template<typename T>
class error : public exception, public T {
public:
  using manipulator = std::ostream& (*)(std::ostream&);

  template<typename... Args>
  explicit error(Args&&... args) : T(std::forward<Args>(args)...)
  {}

  error(error&& other) = default;
  error(const error& other) = default;

  error& operator=(error&& other) = default;
  error& operator=(const error& other) = default;

  template<typename V>
  error& operator<<(V&& v)
  {
    std::ostringstream oss;
    oss.flags(flags_);
    oss << std::forward<V>(v);
    flags_ = oss.flags();
    if (info_) {
      info_->append(oss.str());
    } else {
      info_.emplace(oss.str());
    }
    return *this;
  }

  error& operator<<(manipulator manipulator)
  {
    std::ostringstream oss;
    oss.flags(flags_);
    manipulator(oss);
    flags_ = oss.flags();
    if (info_) {
      info_->append(oss.str());
    } else {
      info_.emplace(oss.str());
    }
    return *this;
  }

  const char* what() const noexcept override
  {
    return T::what();
  }

  const char* info() const noexcept override
  {
    if (!info_) {
      return nullptr;
    }
    return info_->c_str();
  }

private:
  static std::ios_base::fmtflags default_flags()
  {
    static const std::ios_base::fmtflags flags = std::ostringstream().flags();
    return flags;
  }

  ice::optional<std::string> info_;
  std::ios_base::fmtflags flags_ = default_flags();
};

using logic_error = ice::error<std::logic_error>;
using invalid_argument = ice::error<std::invalid_argument>;
using domain_error = ice::error<std::domain_error>;
using length_error = ice::error<std::length_error>;
using out_of_range = ice::error<std::out_of_range>;
using runtime_error = ice::error<std::runtime_error>;
using range_error = ice::error<std::range_error>;
using overflow_error = ice::error<std::overflow_error>;
using underflow_error = ice::error<std::underflow_error>;

using system_error = ice::error<std::system_error>;

}  // namespace ice