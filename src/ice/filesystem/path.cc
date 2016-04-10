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

#include <ice/filesystem/path.h>
#include <ice/date.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cctype>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>

#if defined(__linux)
#include <linux/limits.h>
#include <string.h>
#endif

namespace ice {
namespace filesystem {
namespace {

#ifdef _WIN32

struct path_find_handle {
  WIN32_FIND_DATA ffd = {};
  HANDLE find = INVALID_HANDLE_VALUE;
  path_find_handle(std::wstring path)
  {
    find = FindFirstFile((path + L"\\*").c_str(), &ffd);
  }
  ~path_find_handle()
  {
    if (find != INVALID_HANDLE_VALUE) {
      FindClose(find);
    }
  }
};

#endif

}  // namespace

path::path() :
  m_type(native_path), m_absolute(false)
{}

path::path(path&& path) :
  m_type(path.m_type), m_path(std::move(path.m_path)),
  m_absolute(path.m_absolute)
{}

path::path(const path& path) :
  m_type(path.m_type), m_path(path.m_path), m_absolute(path.m_absolute)
{}

path::path(const char* string)
{
  set(string);
}

path::path(const std::string& string)
{
  set(string);
}

#if defined(_WIN32)

path::path(const std::wstring& wstring)
{
  set(wstring);
}

path::path(const wchar_t* wstring)
{
  set(wstring);
}

#endif

path& path::operator=(path&& path)
{
  if (this != &path) {
    m_type = path.m_type;
    m_path = std::move(path.m_path);
    m_absolute = path.m_absolute;
  }
  return *this;
}

path& path::operator=(const path& path)
{
  m_type = path.m_type;
  m_path = path.m_path;
  m_absolute = path.m_absolute;
  return *this;
}

#if defined(_WIN32)

path& path::operator=(const std::wstring& str)
{
  set(str); return *this;
}

#endif

std::size_t path::length() const
{
  return m_path.size();
}

bool path::empty() const
{
  return m_path.empty();
}

bool path::is_absolute() const
{
  return m_absolute;
}

path path::make_absolute() const
{
#if !defined(_WIN32)
  char temp[PATH_MAX];
  if (realpath(str().c_str(), temp) == NULL) {
    throw std::runtime_error("Internal error in realpath(): " + std::string(strerror(errno)));
  }
  return path(temp);
#else
  std::wstring value = wstr(), out(MAX_PATH, '\0');
  DWORD length = GetFullPathNameW(value.c_str(), MAX_PATH, &out[0], NULL);
  if (length == 0) {
    throw std::runtime_error("Internal error in realpath(): " + std::to_string(GetLastError()));
  }
  return path(out.substr(0, length));
#endif
}

bool path::exists() const
{
#if defined(_WIN32)
  return GetFileAttributesW(wstr().c_str()) != INVALID_FILE_ATTRIBUTES;
#else
  struct stat sb;
  return stat(str().c_str(), &sb) == 0;
#endif
}

std::size_t path::file_size() const
{
#if defined(_WIN32)
  struct _stati64 sb;
  if (_wstati64(wstr().c_str(), &sb) != 0) {
    throw std::runtime_error("path::file_size(): cannot stat file \"" + str() + "\"!");
  }
#else
  struct stat sb;
  if (stat(str().c_str(), &sb) != 0) {
    throw std::runtime_error("path::file_size(): cannot stat file \"" + str() + "\"!");
  }
#endif
  return static_cast<std::size_t>(sb.st_size);
}

bool path::is_directory() const
{
#if defined(_WIN32)
  DWORD result = GetFileAttributesW(wstr().c_str());
  if (result == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (result & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  struct stat sb;
  if (stat(str().c_str(), &sb)) {
    return false;
  }
  return S_ISDIR(sb.st_mode);
#endif
}

bool path::is_file() const
{
#if defined(_WIN32)
  DWORD attr = GetFileAttributesW(wstr().c_str());
  return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
  struct stat sb;
  if (stat(str().c_str(), &sb)) {
    return false;
  }
  return S_ISREG(sb.st_mode);
#endif
}

std::string path::extension() const
{
  const std::string& name = filename();
  std::size_t pos = name.find_last_of(".");
  if (pos == std::string::npos) {
    return "";
  }
  return name.substr(pos + 1);
}

std::string path::filename() const
{
  if (empty()) {
    return "";
  }
  const std::string& last = m_path[m_path.size() - 1];
  return last;
}

path path::parent_path() const
{
  path result;
  result.m_absolute = m_absolute;
  if (m_path.empty()) {
    if (!m_absolute) {
      result.m_path.push_back("..");
    }
  } else {
    std::size_t until = m_path.size() - 1;
    for (std::size_t i = 0; i < until; ++i) {
      result.m_path.push_back(m_path[i]);
    }
  }
  return result;
}

path path::operator/(const path& other) const
{
  if (other.m_absolute) {
    throw std::runtime_error("path::operator/(): expected a relative path");
  }
  if (m_type != other.m_type) {
    throw std::runtime_error("path::operator/(): expected a path of the same type");
  }
  path result(*this);
  for (std::size_t i = 0; i < other.m_path.size(); ++i) {
    result.m_path.push_back(other.m_path[i]);
  }
  return result;
}

void path::set(const std::string& str, path_type type)
{
  m_type = type;
  if (type == windows_path) {
    m_path = tokenize(str, "/\\");
    m_absolute = str.size() >= 2 && std::isalpha(str[0]) && str[1] == ':';
  } else {
    m_path = tokenize(str, "/");
    m_absolute = !str.empty() && str[0] == '/';
  }
}

#if defined(_WIN32)

void path::set(const std::wstring& wstring, path_type type)
{
  std::string string;
  if (!wstring.empty()) {
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstring[0], (int)wstring.size(), NULL, 0, NULL, NULL);
    string.resize(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstring[0], (int)wstring.size(), &string[0], size, NULL, NULL);
  }
  set(string, type);
}

#endif

std::string path::str(path_type type) const
{
  std::ostringstream oss;
  if (m_type == posix_path && m_absolute) {
    oss << "/";
}
  for (std::size_t i = 0; i < m_path.size(); ++i) {
    oss << m_path[i];
    if (i + 1 < m_path.size()) {
      if (type == posix_path) {
        oss << '/';
      } else {
        oss << '\\';
      }
    }
  }
  return oss.str();
}

#if defined(_WIN32)

std::wstring path::wstr(path_type type) const
{
  std::string temp = str(type);
  int size = MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), NULL, 0);
  std::wstring result(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, &temp[0], (int)temp.size(), &result[0], size);
  return result;
}

#endif

bool path::remove_file()
{
#if !defined(_WIN32)
  return std::remove(str().c_str()) == 0;
#else
  return DeleteFileW(wstr().c_str()) != 0;
#endif
}

bool path::resize_file(std::size_t target_length)
{
#if !defined(_WIN32)
  return ::truncate(str().c_str(), (off_t)target_length) == 0;
#else
  HANDLE handle = CreateFileW(wstr().c_str(), GENERIC_WRITE, 0, nullptr, 0, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  LARGE_INTEGER size;
  size.QuadPart = (LONGLONG)target_length;
  if (SetFilePointerEx(handle, size, NULL, FILE_BEGIN) == 0) {
    CloseHandle(handle);
    return false;
  }
  if (SetEndOfFile(handle) == 0) {
    CloseHandle(handle);
    return false;
  }
  CloseHandle(handle);
  return true;
#endif
}

path path::getcwd()
{
#if !defined(_WIN32)
  char temp[PATH_MAX];
  if (::getcwd(temp, PATH_MAX) == NULL) {
    throw std::runtime_error("Internal error in getcwd(): " + std::string(strerror(errno)));
  }
  return path(temp);
#else
  std::wstring temp(MAX_PATH, '\0');
  if (!_wgetcwd(&temp[0], MAX_PATH)) {
    throw std::runtime_error("Internal error in getcwd(): " + std::to_string(GetLastError()));
  }
  return path(temp.c_str());
#endif
}

void path::list(std::function<bool(const ice::filesystem::path& entry)> handler) const
{
  if (!is_directory()) {
    throw std::runtime_error("filesystem: not a directory: " + str());
  }
#ifdef _WIN32
  path_find_handle pfh(wstr());
  if (pfh.find != INVALID_HANDLE_VALUE) {
    do {
      std::wstring name(pfh.ffd.cFileName);
      if (name != L"." && name != L"..") {
        if (!handler(*this / name)) {
          break;
        }
      }
    } while (FindNextFile(pfh.find, &pfh.ffd) != 0);
  }
#else
#error TODO: Implement me!
#endif
}

path::clock::time_point path::modified() const
{
#ifdef _MSC_VER
  auto wfile = make_absolute().wstr();
  auto handle = CreateFile(wfile.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return{};
  }

  FILETIME ft = {};
  auto ok = GetFileTime(handle, nullptr, nullptr, &ft);
  CloseHandle(handle);

  SYSTEMTIME st = {};
  if (ok && FileTimeToSystemTime(&ft, &st)) {
    auto day = ice::date::year(st.wYear) / ice::date::month(st.wMonth) / ice::date::day(st.wDay);
    auto tod = std::chrono::hours(st.wHour) + std::chrono::minutes(st.wMinute) + std::chrono::seconds(st.wSecond);
    return clock::time_point(ice::date::day_point(day)) + tod;
  }
#else
  struct stat attrib;
  if (stat(make_absolute().str().c_str(), &attrib) == 0) {
    return clock::from_time_t(attrib.st_mtime);
  }
#endif
  throw std::runtime_error("Could not get the last modified date/time.");
  return{};
}

std::vector<std::string> path::tokenize(const std::string& string, const std::string& delim)
{
  std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
  std::vector<std::string> tokens;
  while (lastPos != std::string::npos) {
    if (pos != lastPos) {
      tokens.push_back(string.substr(lastPos, pos - lastPos));
    }
    lastPos = pos;
    if (lastPos == std::string::npos || lastPos + 1 == string.length()) {
      break;
    }
    pos = string.find_first_of(delim, ++lastPos);
  }
  return tokens;
}

bool create_directory(const path& p)
{
#if defined(_WIN32)
  return CreateDirectoryW(p.wstr().c_str(), NULL) != 0;
#else
  return mkdir(p.str().c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
#endif
}

}  // namespace filesystem
}  // namespace ice
