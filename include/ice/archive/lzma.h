#pragma once
#include <memory>

namespace ice {
namespace archive {

class lzma {
public:
  lzma();

  lzma(lzma&& other);
  lzma& operator=(lzma&& other);

  ~lzma();

private:
  class impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace archive
}  // namespace ice