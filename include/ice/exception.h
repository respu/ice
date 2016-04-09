#pragma once

namespace ice {

class exception {
public:
  // Returns the error message from the original exception.
  virtual const char* what() const noexcept = 0;

  // Returns additional information or nullptr.
  virtual const char* info() const noexcept = 0;
};

}  // namespace ice