#include <ice/log.h>
#include <ice/filesystem/path.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <atomic>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace ice {
namespace log {
namespace {

#ifdef _WIN32
HANDLE windows_cout = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE windows_cerr = GetStdHandle(STD_ERROR_HANDLE);
#endif

log::severity g_threshold = log::severity::debug;

std::ostream& format_timestamp(std::ostream& os, const log::timestamp timestamp, bool milliseconds)
{
  auto time = std::chrono::system_clock::to_time_t(timestamp);
  tm tm = { 0 };
#ifndef _WIN32
  localtime_r(&time, &tm);
#else
  localtime_s(&tm, &time);
#endif
  os << std::put_time(&tm, "%F %T");
  if (milliseconds) {
    auto tsp = timestamp.time_since_epoch();
    auto s = std::chrono::duration_cast<std::chrono::seconds>(tsp).count();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tsp).count() - (s * 1000);
    os << '.' << std::setfill('0') << std::setw(3) << ms;
  }
  return os;
}

std::ostream& format_severity(std::ostream& os, const log::severity severity)
{
  switch (severity) {
  case log::severity::emergency: return os << "emergency"; break;
  case log::severity::alert:     return os << "alert    "; break;
  case log::severity::critical:  return os << "critical "; break;
  case log::severity::error:     return os << "error    "; break;
  case log::severity::warning:   return os << "warning  "; break;
  case log::severity::notice:    return os << "notice   "; break;
  case log::severity::info:      return os << "info     "; break;
  case log::severity::debug:     return os << "debug    "; break;
  }
  return os << "unknown  ";
}

#ifndef _WIN32

void set_color(std::ostream& os, log::severity severity)
{
  // #!/bin/sh
  // for x in 0 1 4 5 7 8; do
  //   for i in `seq 30 37`; do
  //     for a in `seq 40 47`; do
  //       echo -ne "\e[$x;$i;$a""m\\\e[$x;$i;$a""m\e[0;37;40m "
  //     done
  //     echo
  //   done
  // done
  switch (severity) {
  case log::severity::emergency: os << "\e[0;36m"; break;  // cyan
  case log::severity::alert:     os << "\e[0;34m"; break;  // blue
  case log::severity::critical:  os << "\e[0;35m"; break;  // magenta
  case log::severity::error:     os << "\e[0;31m"; break;  // red
  case log::severity::warning:   os << "\e[0;33m"; break;  // yellow
  case log::severity::notice:    os << "\e[0;32m"; break;  // green
  case log::severity::info:      os << "\e[0;37m"; break;  // white
  case log::severity::debug:     os << "\e[1;30m"; break;  // grey (bold)
  }
}

void reset_color(std::ostream& os)
{
  os << "\e[0m";
}

#else

WORD set_color(HANDLE windows_os, log::severity severity)
{
  CONSOLE_SCREEN_BUFFER_INFO console_info = {};
  GetConsoleScreenBufferInfo(windows_os, &console_info);
  switch (severity) {
  case log::severity::emergency: SetConsoleTextAttribute(windows_os, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;  // cyan
  case log::severity::alert:     SetConsoleTextAttribute(windows_os, FOREGROUND_BLUE | FOREGROUND_INTENSITY); break;  // blue
  case log::severity::critical:  SetConsoleTextAttribute(windows_os, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY); break;  // magenta
  case log::severity::error:     SetConsoleTextAttribute(windows_os, FOREGROUND_RED | FOREGROUND_INTENSITY); break;  // red
  case log::severity::warning:   SetConsoleTextAttribute(windows_os, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;  // yellow
  case log::severity::notice:    SetConsoleTextAttribute(windows_os, FOREGROUND_GREEN | FOREGROUND_INTENSITY); break;  // green
  case log::severity::info:      SetConsoleTextAttribute(windows_os, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); break;  // white
  case log::severity::debug:     SetConsoleTextAttribute(windows_os, FOREGROUND_INTENSITY); break;  // grey (bold)
  }
  return console_info.wAttributes;
}

void reset_color(HANDLE windows_os, WORD attributes)
{
  SetConsoleTextAttribute(windows_os, attributes);
}

#endif

class logger {
public:
  ~logger()
  {
    stop(std::chrono::seconds(0));
  }

  void start()
  {
    std::lock_guard<std::mutex> lock(thread_mutex_);
    if (!running_.exchange(true)) {
      thread_ = std::thread([this]()
      {
        run();
      });
    }
  }

  void stop(clock::duration timeout)
  {
    end_ = clock::now() + timeout;
    if (running_.exchange(false) && thread_.joinable()) {
      cv_.notify_one();
      thread_.join();
    }
  }

  void write(ice::log::message message)
  {
    if (running_) {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      queue_.push_back(std::move(message));
      cv_.notify_all();
    }
  }

  void add(std::shared_ptr<ice::log::sink> sink)
  {
    if (sink) {
      std::lock_guard<std::mutex> lock(sinks_mutex_);
      sinks_.insert(std::move(sink));
    }
  }

  void remove(std::shared_ptr<ice::log::sink> sink)
  {
    if (sink) {
      std::lock_guard<std::mutex> lock(sinks_mutex_);
      sinks_.erase(sink);
    }
  }

private:
  void run()
  {
    while (true) {
      ice::log::message message;
      {
        std::unique_lock<std::mutex> queue_lock(queue_mutex_);
        while (true) {
          if (queue_.empty()) {
            if (!running_) {
              // Exit due to a stop request and an empty queue.
              return;
            }
            // Wait for a notification.
            cv_.wait(queue_lock);
          } else {
            if (!running_ && clock::now() > end_) {
              // Exit due to a stop request and a reached timeout.
              return;
            }
            // Handle message.
            break;
          }
        }
        message = std::move(queue_.front());
        queue_.pop_front();
      }
      std::lock_guard<std::mutex> sinks_lock(sinks_mutex_);
      for (auto& sink : sinks_) {
        try {
          sink->write(message);
        }
        catch (...) {}
      }
    }
  }

  std::thread thread_;
  std::mutex thread_mutex_;

  std::set<std::shared_ptr<sink>> sinks_;
  std::mutex sinks_mutex_;

  std::deque<log::message> queue_;
  std::mutex queue_mutex_;

  std::atomic<bool> running_ = { false };
  log::clock::time_point end_;
  std::condition_variable cv_;
};

logger& g_logger()
{
  static logger logger;
  return logger;
}

}  // namespace

std::ostream& operator<<(std::ostream& os, log::severity severity)
{
  switch (severity) {
  case log::severity::emergency: return os << "emergency"; break;
  case log::severity::alert:     return os << "alert"; break;
  case log::severity::critical:  return os << "critical"; break;
  case log::severity::error:     return os << "error"; break;
  case log::severity::warning:   return os << "warning"; break;
  case log::severity::notice:    return os << "notice"; break;
  case log::severity::info:      return os << "info"; break;
  case log::severity::debug:     return os << "debug"; break;
  }
  return os << static_cast<int>(severity);
}

void start()
{
  g_logger().start();
}

void stop(clock::duration timeout)
{
  g_logger().stop(std::move(timeout));
}

void threshold(log::severity threshold)
{
  g_threshold = threshold;
}

log::severity threshold()
{
  return g_threshold;
}

stream::stream(log::severity severity) :
  std::stringbuf(), std::ostream(this), severity_(severity)
{}

stream::~stream()
{
  try {
    if (severity_ <= g_threshold) {
      ice::log::message message = {
        severity_, std::move(timestamp_), str()
      };
      auto pos = message.text.find_last_not_of(" \t\n\v\f\r");
      if (pos != std::string::npos) {
        message.text.erase(pos + 1);
      }
      if (!message.text.empty()) {
        g_logger().write(std::move(message));
      }
    }
  }
  catch (...) {
  }
}

void add(std::shared_ptr<ice::log::sink> sink)
{
  g_logger().add(std::move(sink));
}
void remove(std::shared_ptr<ice::log::sink> sink)
{
  g_logger().remove(std::move(sink));
}

console::console(log::severity severity, bool milliseconds) :
  severity_(severity), milliseconds_(milliseconds)
{}

void console::write(const ice::log::message& message)
{
  if (message.severity > severity_) {
    return;
  }
  std::ostream& os = message.severity < log::severity::warning ? std::cerr : std::cout;

#ifdef _WIN32
  auto windows_os = message.severity < log::severity::warning ? windows_cerr : windows_cout;
#endif

  // Print the timestamp.
  format_timestamp(os, message.timestamp, milliseconds_);

  // Print the severity opening bracket.
  os << " [";

  // Switch color.
#ifndef _WIN32
  set_color(os, message.severity);
#else
  auto attributes = set_color(windows_os, message.severity);
#endif

  // Print the severity.
  format_severity(os, message.severity);

  // Reset the color.
#ifndef _WIN32
  reset_color(os);
#else
  reset_color(windows_os, attributes);
#endif

  // Print the severity closing bracket.
  os << "] ";

  // Switch color.
#ifndef _WIN32
  set_color(os, message.severity);
#else
  attributes = set_color(windows_os, message.severity);
#endif

  // Print the message.
  os << message.text;

  // Reset the color.
#ifndef _WIN32
  reset_color(os);
#else
  reset_color(windows_os, attributes);
#endif

  // Print the endline sequence and flush the output.
  os << std::endl;
}

class file::impl {
public:
  impl(const ice::filesystem::path& path, bool append) :
#ifdef _WIN32
    os_(path.wstr(), std::ios::binary | (append ? std::ios::app : 0))
#else
    os_(path.str(), std::ios::binary | (append ? std::ios::app : 0))
#endif
  {
    if (!os_.good()) {
      throw std::runtime_error("Could not open log file: " + path.str());
    }
  }

  void write(const log::message& message, bool milliseconds)
  {
    format_timestamp(os_, message.timestamp, milliseconds) << " [";
    format_severity(os_, message.severity) << "] " << message.text << std::endl;
  }

private:
  std::ofstream os_;
};

file::file(const std::string& path, log::severity severity, bool milliseconds, bool append) :
  impl_(std::make_unique<impl>(ice::filesystem::path(path), append)), severity_(severity), milliseconds_(milliseconds)
{}

void file::write(const log::message& message)
{
  if (message.severity <= severity_) {
    impl_->write(message, milliseconds_);
  }
}

}  // namespace log
}  // namespace ice
