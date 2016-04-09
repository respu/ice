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
#include <ostream>
#include <string>
#include <vector>
#include <cstdint>

namespace ice {
namespace filesystem {

class path {
public:
  enum path_type {
    windows_path = 0,
    posix_path = 1,
#if defined(_WIN32)
    native_path = windows_path
#else
    native_path = posix_path
#endif
  };

  path();
  path(path&& path);
  path(const path& path);

  path(const char* string);
  path(const std::string& string);

#if defined(_WIN32)
  path(const std::wstring& wstring);
  path(const wchar_t* wstring);
#endif

  path& operator=(path&& path);
  path& operator=(const path& path);

#if defined(_WIN32)
  path& operator=(const std::wstring& str);
#endif

  std::size_t length() const;
  bool empty() const;
  bool is_absolute() const;
  path make_absolute() const;
  bool exists() const;
  std::size_t file_size() const;
  bool is_directory() const;
  bool is_file() const;
  std::string extension() const;
  std::string filename() const;
  path parent_path() const;
  path operator/(const path& other) const;
  void set(const std::string& str, path_type type = native_path);

  std::string str(path_type type = native_path) const;

#if defined(_WIN32)
  void set(const std::wstring& wstring, path_type type = native_path);
#endif

#if defined(_WIN32)
  std::wstring wstr(path_type type = native_path) const;
#endif

  friend std::ostream& operator<<(std::ostream& os, const path& path)
  {
    os << path.str();
    return os;
  }

  bool remove_file();
  bool resize_file(std::size_t target_length);
  static path getcwd();

protected:
  static std::vector<std::string> tokenize(const std::string& string, const std::string& delim);

protected:
  path_type m_type;
  std::vector<std::string> m_path;
  bool m_absolute;
};

bool create_directory(const path& p);

}  // namespace filesystem
}  // namespace ice