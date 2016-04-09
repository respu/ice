#include <ice/windows/error.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <string>

namespace ice {
namespace windows {

class error_category_impl : public std::error_category {
public:
  virtual const char* name() const noexcept override
  {
    return "ice::windows::error";
  }

  virtual std::string message(int error) const override
  {
#ifdef _WIN32
    DWORD e = static_cast<DWORD>(error);
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
    LPTSTR str = nullptr;
    auto size = FormatMessage(flags, nullptr, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&str), 0, nullptr);
    if (size > 0 && str) {
      std::string msg;
      msg.resize(WideCharToMultiByte(CP_UTF8, 0, str, static_cast<int>(size), nullptr, 0, nullptr, nullptr) + 1);
      msg.resize(WideCharToMultiByte(CP_UTF8, 0, str, static_cast<int>(size), &msg[0], static_cast<int>(msg.size()), nullptr, nullptr));
      LocalFree(str);
      return msg;
    }
#endif
    return std::to_string(error);
  }
};

const std::error_category& error_category()
{
  static error_category_impl instance;
  return instance;
}

std::error_code make_error()
{
#ifdef _WIN32
  return make_error(GetLastError());
#else
  return make_error(0);
#endif
}

std::error_code make_error(unsigned e)
{
  return std::error_code(static_cast<int>(e), error_category());
}

}  // namespace windows
}  // namespace ice