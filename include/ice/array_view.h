// This implementation is adapted from libcxx.
//
// Copyright (c) 2015, Dahua Lin
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace ice {

template <typename T>
class array_view {
  static_assert(::std::is_object<T>::value,
    "array_view<T>: T must be an object");

public:
  // types
  typedef typename ::std::remove_cv<T>::type value_type;
  typedef ::std::size_t size_type;
  typedef ::std::ptrdiff_t difference_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;

  typedef pointer iterator;
  typedef const_pointer const_iterator;
  typedef ::std::reverse_iterator<iterator> reverse_iterator;
  typedef ::std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
  pointer data_;
  size_type len_;

public:
  // constructors, destructor, assignment operator

  constexpr array_view() noexcept :
    data_(nullptr), len_(0)
  {}

  constexpr array_view(pointer data, size_type len) noexcept :
    data_(data), len_(len)
  {}

  constexpr array_view(const array_view&) noexcept = default;

  ~array_view() noexcept = default;

  array_view& operator=(const array_view&) noexcept = default;

  // swap

  void swap(array_view& other) noexcept
  {
    ::std::swap(data_, other.data_);
    ::std::swap(len_, other.len_);
  }

  // size related

  constexpr bool empty() const noexcept { return len_ == 0; }
  constexpr size_type size() const noexcept { return len_; }
  constexpr size_type max_size() const noexcept
  {
    return std::numeric_limits<size_type>::max();
  }

  // element access

  constexpr const_pointer data() const noexcept { return data_; }
  pointer data() noexcept { return data_; }

  constexpr const_reference at(size_type pos) const
  {
    return pos < len_ ? data_[pos] :
      (throw ::std::out_of_range("basic_string_view::at"), data_[0]);
  }

  reference at(size_type pos)
  {
    return pos < len_ ? data_[pos] :
      (throw ::std::out_of_range("basic_string_view::at"), data_[0]);
  }

  constexpr const_reference operator[](size_type pos) const
  {
    return data_[pos];
  }

  reference operator[](size_type pos)
  {
    return data_[pos];
  }

  constexpr const_reference front() const
  {
    return data_[0];
  }

  reference front()
  {
    return data_[0];
  }

  constexpr const_reference back() const
  {
    return data_[len_ - 1];
  }

  reference back()
  {
    return data_[len_ - 1];
  }

  // iterators

  iterator begin() noexcept { return data_; }
  iterator end() noexcept { return data_ + len_; }
  constexpr const_iterator begin() const noexcept { return data_; }
  constexpr const_iterator end() const noexcept { return data_ + len_; }
  constexpr const_iterator cbegin() const noexcept { return begin(); }
  constexpr const_iterator cend() const noexcept { return end(); }

  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
  constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
  constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  constexpr const_reverse_iterator crend() const noexcept { return rend(); }
};

template <typename T>
constexpr array_view<T> aview(T* p, ::std::size_t n) noexcept
{
  return array_view<T>(p, n);
}

template <typename T>
inline void swap(array_view<T>& a, array_view<T>& b) noexcept
{
  a.swap(b);
}

}  // namespace ice