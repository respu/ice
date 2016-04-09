// Copyright (c) 2015, Wenzel Jakob
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <ice/filesystem/path.h>
#include <vector>

namespace ice {
namespace filesystem {

class resolver {
public:
  typedef std::vector<path>::iterator iterator;
  typedef std::vector<path>::const_iterator const_iterator;

  resolver()
  {
    m_paths.push_back(path::getcwd());
  }

  std::size_t size() const
  {
    return m_paths.size();
  }

  iterator begin()
  {
    return m_paths.begin();
  }

  iterator end()
  {
    return m_paths.end();
  }

  const_iterator begin() const
  {
    return m_paths.begin();
  }

  const_iterator end() const
  {
    return m_paths.end();
  }

  void erase(iterator it)
  {
    m_paths.erase(it);
  }

  void prepend(const path& path)
  {
    m_paths.insert(m_paths.begin(), path);
  }

  void append(const path& path)
  {
    m_paths.push_back(path);
  }

  const path& operator[](std::size_t index) const
  {
    return m_paths[index];
  }

  path& operator[](std::size_t index)
  {
    return m_paths[index];
  }

  path resolve(const path& value) const
  {
    for (const_iterator it = m_paths.begin(); it != m_paths.end(); ++it) {
      path combined = *it / value;
      if (combined.exists()) {
        return combined;
      }
    }
    return value;
  }

  friend std::ostream& operator<<(std::ostream& os, const resolver& r)
  {
    os << "resolver[" << std::endl;
    for (std::size_t i = 0; i < r.m_paths.size(); ++i) {
      os << "  \"" << r.m_paths[i] << "\"";
      if (i + 1 < r.m_paths.size()) {
        os << ",";
      }
      os << std::endl;
    }
    os << "]";
    return os;
  }

private:
  std::vector<path> m_paths;
};

}  // namespace filesystem
}  // namespace ice