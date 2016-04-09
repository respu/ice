#include <ice/application.h>
#include <ice/error.h>
#include <ice/windows/error.h>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <unistd.h>
#endif
#include <fstream>
#include <string>
#include <clocale>

namespace ice {
namespace application {

ice::filesystem::path file()
{
  std::error_code ec;
  auto application_file = file(ec);
  if (ec) {
    throw ice::system_error(ec)
      << "Could not get the application filename.";
  }
  return application_file;
}

ice::filesystem::path file(std::error_code& ec)
{
  // http://www.tech.theplayhub.com/finding_current_executables_path_without_procselfexe-7
  // =====================================================================================
  // Mac OS X: _NSGetExecutablePath() (man 3 dyld)
  // Linux: readlink /proc/self/exe
  // Solaris: getexecname()
  // FreeBSD: sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
  // FreeBSD if it has procfs: readlink /proc/curproc/file (FreeBSD doesn’t have procfs by default)
  // NetBSD: readlink /proc/curproc/exe
  // DragonFly BSD: readlink /proc/curproc/file
  // Windows: GetModuleFileName() with hModule = NULL
  static ice::filesystem::path path;
#ifdef _WIN32
  if (path.empty()) {
    std::wstring str;
    DWORD size = 0;
    DWORD error = 0;
    do {
      str.resize(str.size() + MAX_PATH);
      size = GetModuleFileName(nullptr, &str[0], static_cast<DWORD>(str.size()));
      error = GetLastError();
    } while (error == ERROR_INSUFFICIENT_BUFFER);
    if (error != 0) {
      ec = ice::windows::make_error(error);
      return path;
    }
    str.resize(size);
    path = ice::filesystem::path(str).make_absolute();
  }
#else
  std::ifstream is("/proc/self/exe", std::ios::binary);
  if (is) {
    std::string str;
    if (std::getline(is, str)) {
      path = str;
    }
  } else {
    ec = std::make_error_code(std::errc::bad_file_descriptor);
  }
#endif
  return path;
}

ice::filesystem::path path()
{
  std::error_code ec;
  auto application_path = path(ec);
  if (ec) {
    throw ice::system_error(ec)
      << "Could not get the application directory.";
  }
  return application_path;
}

ice::filesystem::path path(std::error_code& ec)
{
  static ice::filesystem::path path;
  if (path.empty()) {
    path = file(ec).parent_path();
  }
  return path;
}

}  // namespace application
}  // namespace ice
