#pragma once
#include <ice/date/date.h>
#include <ice/date/tz.h>
#include <ice/date/week.h>
#include <chrono>
#include <limits>
#include <string>
#include <cmath>
#include <cstdio>

namespace ice {
namespace date {

template <class T>
struct is_duration : std::false_type {};

template <class Rep, class Period>
struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};

template <class T>
struct is_time_point : std::false_type {};

template <typename Clock, typename Duration>
struct is_time_point<std::chrono::time_point<Clock, Duration>> : std::true_type {};

template <class T>
struct is_time_of_day : std::false_type {};

template <class Duration>
struct is_time_of_day<date::time_of_day<Duration>> : std::true_type {};

// ====================================================================================================================
// Durations.
// ====================================================================================================================

// Formats the duration as milliseconds.
template <typename Rep, typename Period>
inline std::string format(const std::chrono::duration<Rep, Period>& duration)
{
  return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}

// Parses the duration as milliseconds.
template <typename T>
inline std::enable_if_t<is_duration<T>::value, T> parse(const std::string& str)
{
  long long ms = 0;
  if (std::sscanf(str.data(), "%lld", &ms) != 1) {
    throw std::runtime_error("date: parse error for duration: " + str);
  }
  return std::chrono::duration_cast<T>(std::chrono::milliseconds(ms));
}

// ====================================================================================================================
// Time of day.
// ====================================================================================================================

// Formats the time of day "[-]00:00:00.000".
template <typename Duration>
inline std::string format(const date::time_of_day<Duration>& tod)
{
  auto hours = tod.hours();
  if (tod.mode() != 0) {
    hours = date::time_of_day<Duration>(tod).make24().hours();
  }
  auto h = hours.count();
  auto m = static_cast<unsigned>(std::abs(tod.minutes().count()));
  auto s = static_cast<unsigned>(std::abs(tod.seconds().count()));
  auto ms = static_cast<unsigned>(std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(tod.subseconds()).count()));
  // [-] + "%d" + ':' + "00" + ':' + "00" + '.' + "000" + '\0'
  int size = 1 + 1 + 2 + 1 + 2 + 1 + 3 + 1;
  if (h < 0) {
    size += std::numeric_limits<decltype(h)>::digits10;
  } else if (h < 10) {
    size += 1;
  } else if (h < 100) {
    size += 2;
  } else if (h < 1000) {
    size += 3;
  } else if (h < 10000) {
    size += 4;
  } else if (h < 100000) {
    size += 5;
  } else if (h < 1000000) {
    size += 6;
  } else if (h < 1000000) {
    size += 7;
  } else if (h < 1000000) {
    size += 8;
  } else if (h < 1000000) {
    size += 9;
  } else {
    size += std::numeric_limits<decltype(h)>::digits10;
  }
  std::string str;
  str.resize(static_cast<std::size_t>(size));
  size = std::snprintf(&str[0], str.size(), "%02d:%02u:%02u.%03u", h, m, s, ms);
  if (size < 0) {
    throw std::runtime_error("date: format error for time_of_day: " +
      std::to_string(h) + ':' + std::to_string(m) + ':' + std::to_string(s) + '.' + std::to_string(ms));
  }
  str.resize(size);
  return str;
}

// Parses the time of day as "[-]00:00:00.000".
template <typename T>
inline std::enable_if_t<is_time_of_day<T>::value, T> parse(const std::string& str)
{
  int h = 0;
  unsigned m = 0;
  unsigned s = 0;
  unsigned ms = 0;
  if (str.size() < 12 || std::sscanf(str.data(), "%02d:%02u:%02u.%03u", &h, &m, &s, &ms) != 4) {
    throw std::runtime_error("date: parse error for time_of_day: " + str);
  }
  if (h >= 0) {
    return T(std::chrono::hours(h) + std::chrono::minutes(m) + std::chrono::seconds(s) + std::chrono::milliseconds(ms));
  } else {
    return T(std::chrono::hours(h) - std::chrono::minutes(m) - std::chrono::seconds(s) - std::chrono::milliseconds(ms));
  }
}

// ====================================================================================================================
// Day, weekday, month and year.
// ====================================================================================================================

// Formats the day of a month as "01".
inline std::string format(const date::day& day)
{
  std::string str;
  str.resize(3);
  auto size = std::snprintf(&str[0], str.size(), "%02u", static_cast<unsigned>(day));
  if (size < 0) {
    throw std::runtime_error("date: format error for day: " + std::to_string(static_cast<unsigned>(day)));
  }
  str.resize(size);
  return str;
}

// Parses the day of a month as "01".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::day>::value, T> parse(const std::string& str)
{
  unsigned d = 0;
  if (str.size() != 2 || std::sscanf(str.data(), "%02u", &d) != 1) {
    throw std::runtime_error("date: parse error for day: " + str);
  }
  auto day = date::day(d);
  if (!day.ok()) {
    throw std::runtime_error("date: out of range error for day: " + str);
  }
  return day;
}

// Formats the weekday as "mon".
inline std::string format(const date::weekday& weekday)
{
  auto index = static_cast<unsigned>(weekday);
  switch (index) {
  case 0u: return "sun";
  case 1u: return "mon";
  case 2u: return "tue";
  case 3u: return "wed";
  case 4u: return "thu";
  case 5u: return "fri";
  case 6u: return "sat";
  default: throw std::runtime_error("date: format error for weekday: " + std::to_string(index));
  }
  return std::string();
}

// Parses the weekday as "mon".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::weekday>::value, T> parse(const std::string& str)
{
  if (str.size() != 3) {
    throw std::runtime_error("date: parse error for weekday: " + str);
  }
  if (str.compare(0, 3, "sun") == 0) {
    return date::sun;
  }
  if (str.compare(0, 3, "mon") == 0) {
    return date::mon;
  }
  if (str.compare(0, 3, "tue") == 0) {
    return date::tue;
  }
  if (str.compare(0, 3, "wed") == 0) {
    return date::wed;
  }
  if (str.compare(0, 3, "thu") == 0) {
    return date::thu;
  }
  if (str.compare(0, 3, "fri") == 0) {
    return date::fri;
  }
  if (str.compare(0, 3, "sat") == 0) {
    return date::sat;
  }
  throw std::runtime_error("date: parse error for weekday: " + str);
  return date::weekday(7u);
}

// Parses the month as "jan".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::month>::value, T> parse(const std::string& str)
{
  if (str.size() != 3) {
    throw std::runtime_error("date: parse error for month: " + str);
  }
  if (str.compare(0, 3, "jan") == 0) {
    return date::jan;
  }
  if (str.compare(0, 3, "feb") == 0) {
    return date::feb;
  }
  if (str.compare(0, 3, "mar") == 0) {
    return date::mar;
  }
  if (str.compare(0, 3, "apr") == 0) {
    return date::apr;
  }
  if (str.compare(0, 3, "may") == 0) {
    return date::may;
  }
  if (str.compare(0, 3, "jun") == 0) {
    return date::jun;
  }
  if (str.compare(0, 3, "jul") == 0) {
    return date::jul;
  }
  if (str.compare(0, 3, "aug") == 0) {
    return date::aug;
  }
  if (str.compare(0, 3, "sep") == 0) {
    return date::sep;
  }
  if (str.compare(0, 3, "oct") == 0) {
    return date::oct;
  }
  if (str.compare(0, 3, "nov") == 0) {
    return date::nov;
  }
  if (str.compare(0, 3, "dec") == 0) {
    return date::dec;
  }
  throw std::runtime_error("date: parse error for month: " + str);
  return date::month(13u);
}

// Formats the month as "jan".
inline std::string format(const date::month& month)
{
  auto index = static_cast<unsigned>(month);
  switch (index) {
  case 1u: return "jan";
  case 2u: return "feb";
  case 3u: return "mar";
  case 4u: return "apr";
  case 5u: return "may";
  case 6u: return "jun";
  case 7u: return "jul";
  case 8u: return "aug";
  case 9u: return "sep";
  case 10u: return "oct";
  case 11u: return "nov";
  case 12u: return "dec";
  }
  throw std::runtime_error("date: format error for month: " + std::to_string(index));
  return std::string();
}

// Formats the year as "1970".
inline std::string format(const date::year& year)
{
  std::string str;
  str.resize(5);
  auto size = std::snprintf(&str[0], str.size(), "%04d", static_cast<int>(year));
  if (size < 0) {
    throw std::runtime_error("date: format error for year: " + std::to_string(static_cast<int>(year)));
  }
  str.resize(size);
  return str;
}

// Parses the year as "1970".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::year>::value, T> parse(const std::string& str)
{
  int y = 0;
  if (str.size() != 4 || std::sscanf(str.data(), "%04d", &y) != 1) {
    throw std::runtime_error("date: parse error for year: " + str);
  }
  auto year = date::year(y);
  if (!year.ok()) {
    throw std::runtime_error("date: out of range error for year: " + str);
  }
  return year;
}

// ====================================================================================================================
// Nth and last weekday of a month.
// ====================================================================================================================

// Formats the Nth weekday of a month as "mon[1]".
inline std::string format(const date::weekday_indexed& wi)
{
  return format(wi.weekday()) + '[' + std::to_string(wi.index()) + ']';
}

// Parses the Nth weekday of a month as "mon[1]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::weekday_indexed>::value, T> parse(const std::string& str)
{
  std::string w;
  w.resize(3);
  unsigned index = 0;
  if (str.size() != 6 || std::sscanf(str.data(), "%c%c%c[%u]", &w[0], &w[1], &w[2], &index) != 4) {
    throw std::runtime_error("date: parse error for weekday_indexed: " + str);
  }
  auto wi = date::weekday_indexed(parse<date::weekday>(w), index);
  if (!wi.ok()) {
    throw std::runtime_error("date: out of range error for weekday_indexed: " + str);
  }
  return wi;
}

// Formats the last weekday of a month as "mon[last]".
inline std::string format(const date::weekday_last& wl)
{
  return format(wl.weekday()) + "[last]";
}

// Parses the last weekday of a month as "mon[last]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::weekday_last>::value, T> parse(const std::string& str)
{
  std::string w;
  w.resize(3);
  if (str.size() != 9 || std::sscanf(str.data(), "%c%c%c[last]", &w[0], &w[1], &w[2]) != 3) {
    throw std::runtime_error("date: parse error for weekday_last: " + str);
  }
  auto wl = date::weekday_last(parse<date::weekday>(w));
  if (!wl.ok()) {
    throw std::runtime_error("date: out of range error for weekday_last: " + str);
  }
  return wl;
}

// ====================================================================================================================
// Nth and last day of a specific month.
// ====================================================================================================================

// Formats the day of the month as "jan/01".
inline std::string format(const date::month_day& md)
{
  return format(md.month()) + '/' + format(md.day());
}

// Parses the day of the month as "jan/01".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::month_day>::value, T> parse(const std::string& str)
{
  std::string m;
  m.resize(3);
  unsigned d = 0;
  if (str.size() != 6 || std::sscanf(str.data(), "%c%c%c/%02u", &m[0], &m[1], &m[2], &d) != 4) {
    throw std::runtime_error("date: parse error for month_day: " + str);
  }
  auto md = date::month_day(parse<date::month>(m), date::day(d));
  if (!md.ok()) {
    throw std::runtime_error("date: out of range error for month_day: " + str);
  }
  return md;
}

// Formats the last day of the month as "jan/last".
inline std::string format(const date::month_day_last& mdl)
{
  return format(mdl.month()) + "/last";
}

// Parses the last day of the month as "jan/last".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::month_day_last>::value, T> parse(const std::string& str)
{
  std::string m;
  m.resize(3);
  if (str.size() != 8 || std::sscanf(str.data(), "%c%c%c/last", &m[0], &m[1], &m[2]) != 3) {
    throw std::runtime_error("date: parse error for month_day_last: " + str);
  }
  auto mdl = date::month_day_last(parse<date::month>(m));
  if (!mdl.ok()) {
    throw std::runtime_error("date: out of range error for month_day_last: " + str);
  }
  return mdl;
}

// ====================================================================================================================
// Nth and last weekday of a specific month.
// ====================================================================================================================

// Formats the Nth weekday of the month as "jan/mon[1]".
inline std::string format(const date::month_weekday& mw)
{
  return format(mw.month()) + '/' + format(mw.weekday_indexed());
}

// Parses the Nth weekday of the month as "jan/mon[1]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::month_weekday>::value, T> parse(const std::string& str)
{
  std::string m;
  m.resize(3);
  std::string w;
  w.resize(3);
  unsigned index = 0;
  if (str.size() != 10 || std::sscanf(str.data(), "%c%c%c/%c%c%c[%u]", &m[0], &m[1], &m[2], &w[0], &w[1], &w[2], &index) != 7) {
    throw std::runtime_error("date: parse error for month_weekday: " + str);
  }
  auto mw = date::month_weekday(parse<date::month>(m), date::weekday_indexed(parse<date::weekday>(w), index));
  if (!mw.ok()) {
    throw std::runtime_error("date: out of range error for month_weekday: " + str);
  }
  return mw;
}

// Formats the last weekday of the month as "jan/mon[last]".
inline std::string format(const date::month_weekday_last& mwl)
{
  return format(mwl.month()) + '/' + format(mwl.weekday_last());
}

// Parses the last weekday of the month as "jan/mon[last]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::month_weekday_last>::value, T> parse(const std::string& str)
{
  std::string m;
  m.resize(3);
  std::string w;
  w.resize(3);
  if (str.size() != 13 || std::sscanf(str.data(), "%c%c%c/%c%c%c[last]", &m[0], &m[1], &m[2], &w[0], &w[1], &w[2]) != 6) {
    throw std::runtime_error("date: parse error for month_weekday_last: " + str);
  }
  auto mwl = date::month_weekday_last(parse<date::month>(m), date::weekday_last(parse<date::weekday>(w)));
  if (!mwl.ok()) {
    throw std::runtime_error("date: out of range error for month_weekday_last: " + str);
  }
  return mwl;
}

// ====================================================================================================================
// ISO 8601 Year, month and day combinations.
// ====================================================================================================================

// Formats the month of the year as "1970-01".
inline std::string format(const date::year_month& ym)
{
  std::string str;
  str.resize(8);
  auto y = static_cast<int>(ym.year());
  auto m = static_cast<unsigned>(ym.month());
  auto size = std::snprintf(&str[0], str.size(), "%04d-%02u", y, m);
  if (size < 0) {
    throw std::runtime_error("date: format error for year_month: " + std::to_string(y) + "-" + std::to_string(m));
  }
  str.resize(size);
  return str;
}

// Parses the month of the year as "1970-01".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::year_month>::value, T> parse(const std::string& str)
{
  int y = 0;
  unsigned m = 0;
  if (str.size() != 7 || std::sscanf(str.data(), "%04d-%02u", &y, &m) != 2) {
    throw std::runtime_error("date: parse error for year_month: " + str);
  }
  auto ym = date::year_month(date::year(y), date::month(m));
  if (!ym.ok()) {
    throw std::runtime_error("date: out of range error for year_month: " + str);
  }
  return ym;
}

// Formats the day of the month of the year as "1970-01-01".
inline std::string format(const date::year_month_day& ymd)
{
  std::string str;
  str.resize(11);
  auto y = static_cast<int>(ymd.year());
  auto m = static_cast<unsigned>(ymd.month());
  auto d = static_cast<unsigned>(ymd.day());
  auto size = std::snprintf(&str[0], str.size(), "%04d-%02u-%02u", y, m, d);
  if (size < 0) {
    throw std::runtime_error("date: format error for year_month_day: " + std::to_string(y) + "-" + std::to_string(m) + "-" + std::to_string(d));
  }
  str.resize(size);
  return str;
}

// Parses the day of the month of the year as "1970-01-01".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::year_month_day>::value, T> parse(const std::string& str)
{
  int y = 0;
  unsigned m = 0;
  unsigned d = 0;
  if (str.size() != 10 || std::sscanf(str.data(), "%04d-%02u-%02u", &y, &m, &d) != 3) {
    throw std::runtime_error("date: parse error for year_month_day: " + str);
  }
  auto ymd = date::year_month_day(date::year(y), date::month(m), date::day(d));
  if (!ymd.ok()) {
    throw std::runtime_error("date: out of range error for year_month_day: " + str);
  }
  return ymd;
}

// ====================================================================================================================
// Last day of a specific year and month.
// ====================================================================================================================

// Formats the last day of the month of the year as "1970-01[last]".
inline std::string format(const date::year_month_day_last& ymdl)
{
  return format(date::year_month(ymdl.year(), ymdl.month())) + "[last]";
}

// Parses the last day of the month of the year as "1970-01[last]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::year_month_day_last>::value, T> parse(const std::string& str)
{
  int y = 0;
  unsigned m = 0;
  if (str.size() != 13 || std::sscanf(str.data(), "%04d-%02u[last]", &y, &m) != 2) {
    throw std::runtime_error("date: parse error for year_month: " + str);
  }
  auto ymdl = date::year_month_day_last(date::year(y), date::month_day_last(date::month(m)));
  if (!ymdl.ok()) {
    throw std::runtime_error("date: out of range error for year_month: " + str);
  }
  return ymdl;
}

// ====================================================================================================================
// Nth and last weekday of a specific year and month.
// ====================================================================================================================

// Formats the Nth weekday of the month of the year as "1970-01/mon[1]".
inline std::string format(const date::year_month_weekday& ymw)
{
  return format(date::year_month(ymw.year(), ymw.month())) + '/' + format(ymw.weekday_indexed());
}

// Parses the Nth weekday of the month of the year as "1970-01/mon[1]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::year_month_weekday>::value, T> parse(const std::string& str)
{
  std::string w;
  w.resize(3);
  int y = 0;
  unsigned m = 0;
  unsigned index = 0;
  if (str.size() != 14 || std::sscanf(str.data(), "%04d-%02u/%c%c%c[%u]", &y, &m, &w[0], &w[1], &w[2], &index) != 6) {
    throw std::runtime_error("date: parse error for year_month_weekday: " + str);
  }
  auto ymw = date::year_month_weekday(date::year(y), date::month(m), date::weekday_indexed(parse<date::weekday>(w), index));
  if (!ymw.ok()) {
    throw std::runtime_error("date: out of range error for year_month_day: " + str);
  }
  return ymw;
}

// Formats the last weekday of the month of the year as "1970-01/mon[last]".
inline std::string format(const date::year_month_weekday_last& ymwl)
{
  return format(date::year_month(ymwl.year(), ymwl.month())) + '/' + format(ymwl.weekday_last());
}

// Parses the last weekday of the month of the year as "1970-01/mon[last]".
template <typename T>
inline std::enable_if_t<std::is_same<T, date::year_month_weekday_last>::value, T> parse(const std::string& str)
{
  std::string w;
  w.resize(3);
  int y = 0;
  unsigned m = 0;
  if (str.size() != 17 || std::sscanf(str.data(), "%04d-%02u/%c%c%c[last]", &y, &m, &w[0], &w[1], &w[2]) != 5) {
    throw std::runtime_error("date: parse error for year_month_weekday_last: " + str);
  }
  auto ymwl = date::year_month_weekday_last(date::year(y), date::month(m), date::weekday_last(parse<date::weekday>(w)));
  if (!ymwl.ok()) {
    throw std::runtime_error("date: out of range error for year_month_weekday_last: " + str);
  }
  return ymwl;
}

// ====================================================================================================================
// Timepoints.
// ====================================================================================================================

// Formats the time point as "1970-01-01 00:00:00.000".
template <typename Clock>
inline std::string format(const std::chrono::time_point<Clock, std::chrono::milliseconds>& tp)
{
  auto dp = date::floor<date::days>(tp);
  return format(date::year_month_day(dp)) + ' ' + format(date::make_time(tp - dp));
}

// Formats the day point as "1970-01-01".
template <typename Clock>
inline std::string format(const std::chrono::time_point<Clock, date::days>& dp)
{
  return format(date::year_month_day(dp));
}

// Parses the time point or day point.
template <typename T>
inline std::enable_if_t<is_time_point<T>::value, T> parse(const std::string& str)
{
  int Y = 0;
  unsigned M = 0;
  unsigned D = 0;
  auto size = str.size();

  if (size == 23) {
    unsigned h = 0;
    unsigned m = 0;
    unsigned s = 0;
    unsigned S = 0;

    if (std::sscanf(str.data(), "%04d-%02u-%02u %02d:%02u:%02u.%03u", &Y, &M, &D, &h, &m, &s, &S) != 7) {
      throw std::runtime_error("date: parse error for time_point: " + str);
    }

    auto cY = date::year(Y);
    auto cM = date::month(M);
    auto cD = date::day(D);

    auto ch = std::chrono::hours(h);
    auto cm = std::chrono::minutes(m);
    auto cs = std::chrono::seconds(s);
    auto cS = std::chrono::milliseconds(S);

    auto dp = static_cast<date::day_point>(date::year_month_day(cY, cM, cD));
    auto tp = dp + date::time_of_day<std::chrono::milliseconds>(ch, cm, cs, cS).to_duration();

    return date::floor<T::duration>(tp);
  } else if (size == 10) {
    if (std::sscanf(str.data(), "%04d-%02u-%02u", &Y, &M, &D) != 3) {
      throw std::runtime_error("date: parse error for day_point: " + str);
    }
    auto cY = date::year(Y);
    auto cM = date::month(M);
    auto cD = date::day(D);

    auto dp = static_cast<date::day_point>(date::year_month_day(cY, cM, cD));
    return date::floor<T::duration>(dp);
  } else {
    throw std::runtime_error("date: parse error for generic time_point: " + str);
  }
  return T();
}

}  // namespace date
}  // namespace ice