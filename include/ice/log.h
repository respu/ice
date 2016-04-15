#pragma once
#include <chrono>
#include <memory>
#include <ostream>
#include <sstream>

namespace ice {
namespace log {

// The clock used by the logging infrastructure.
using clock = std::chrono::system_clock;

// Log message timestamp.
using timestamp = clock::time_point;

// Log message severity.
enum class severity {
  emergency = 0,
  alert = 1,
  critical = 2,
  error = 3,
  warning = 4,
  notice = 5,
  info = 6,
  debug = 7
};

// Log message.
struct message {
  log::severity severity;
  log::timestamp timestamp;
  std::string text;
};

// Starts the logging thread.
void start();

// Stops the logging thread when all messages are processed or the timeout is eached.
void stop(clock::duration timeout = std::chrono::seconds(0));

// Sets the default severity threshold.
void threshold(log::severity threshold);

// Returns the default severity threshold.
log::severity threshold();

// Log stream convenience class for creating and writing log messages.
class stream : public std::stringbuf, public std::ostream {
public:
  stream(log::severity severity);

  stream(stream&& other) = default;
  stream(const stream& other) = delete;

  stream& operator=(stream&& other) = default;
  stream& operator=(const stream& other) = delete;

  virtual ~stream();

private:
  log::severity severity_;
  timestamp timestamp_ = clock::now();
};

class emergency : public stream {
public:
  emergency() : stream(severity::emergency) {}
};

class alert : public stream {
public:
  alert() : stream(severity::alert) {}
};

class critical : public stream {
public:
  critical() : stream(severity::critical) {}
};

class error : public stream {
public:
  error() : stream(severity::error) {}
};

class warning : public stream {
public:
  warning() : stream(severity::warning) {}
};

class notice : public stream {
public:
  notice() : stream(severity::notice) {}
};

class info : public stream {
public:
  info() : stream(severity::info) {}
};

class debug : public stream {
public:
  debug() : stream(severity::debug) {}
};

// Log sink interface.
class sink {
public:
  virtual ~sink() = default;
  virtual void write(const log::message& message) = 0;
};

// Adds the given log sink.
void add(std::shared_ptr<ice::log::sink> sink);

// Removes the given log sink.
void remove(std::shared_ptr<ice::log::sink> sink);

// Console output log sink.
class console : public sink {
public:
  console(log::severity severity = log::severity::debug);

  void write(const log::message& message) override;

private:
  log::severity severity_;
};

}  // namespace log
}  // namespace ice