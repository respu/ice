#include <ice/archive/lzma.h>

namespace ice {
namespace archive {

class lzma::impl {
};

lzma::lzma() :
  impl_(std::make_unique<impl>())
{}

lzma::lzma(lzma&& other) :
  lzma()
{
  impl_.swap(other.impl_);
}

lzma& lzma::operator=(lzma&& other)
{
  impl_.swap(other.impl_);
  return *this;
}

lzma::~lzma()
{}

}  // namespace archive
}  // namespace ice