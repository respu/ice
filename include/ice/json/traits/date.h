#pragma once
#include <ice/json/traits.h>
#include <ice/json/value.h>
#include <ice/date.h>

// XXX This implementation should be redone with the tz.h header.

namespace ice {
namespace detail {

template<typename T>
struct date_traits {
  static void assign(json::value& self, const T& v)
  {
    self = ice::date::format(v);
  }

  static bool is(const json::value& self)
  {
    return self.is<json::string>();
  }

  static T as(const json::value& self)
  {
    return ice::date::parse<T>(self.as<std::string>());
  }
};

}  // namespace detail

// ====================================================================================================================
// Durations.
// ====================================================================================================================

template <typename Rep, typename Period>
struct json_traits<std::chrono::duration<Rep, Period>> {
  static void assign(json::value& self, const std::chrono::duration<Rep, Period>& v)
  {
    self = std::chrono::duration_cast<std::chrono::milliseconds>(v).count();
  }

  static bool is(const json::value& self)
  {
    return self.is<json::number>();
  }

  static std::chrono::duration<Rep, Period> as(const json::value& self)
  {
    auto ms = std::chrono::milliseconds(self.as<std::chrono::milliseconds::rep>());
    return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(ms);
  }
};

// ====================================================================================================================
// Time of day.
// ====================================================================================================================

template <typename Duration>
struct json_traits<ice::date::time_of_day<Duration>> : detail::date_traits<ice::date::time_of_day<Duration>> {};

// ====================================================================================================================
// Day, weekday, month and year.
// ====================================================================================================================

template <>
struct json_traits<ice::date::day> : detail::date_traits<ice::date::day> {};

template <>
struct json_traits<ice::date::weekday> : detail::date_traits<ice::date::weekday> {};

template <>
struct json_traits<ice::date::month> : detail::date_traits<ice::date::month> {};

template <>
struct json_traits<ice::date::year> : detail::date_traits<ice::date::year> {};

// ====================================================================================================================
// Nth and last weekday of a month.
// ====================================================================================================================

template <>
struct json_traits<ice::date::weekday_indexed> : detail::date_traits<ice::date::weekday_indexed> {};

template <>
struct json_traits<ice::date::weekday_last> : detail::date_traits<ice::date::weekday_last> {};

// ====================================================================================================================
// Nth and last day of a specific month.
// ====================================================================================================================

template <>
struct json_traits<ice::date::month_day> : detail::date_traits<ice::date::month_day> {};

template <>
struct json_traits<ice::date::month_day_last> : detail::date_traits<ice::date::month_day_last> {};

// ====================================================================================================================
// Nth and last weekday of a specific month.
// ====================================================================================================================

template <>
struct json_traits<ice::date::month_weekday> : detail::date_traits<ice::date::month_weekday> {};

template <>
struct json_traits<ice::date::month_weekday_last> : detail::date_traits<ice::date::month_weekday_last> {};

// ====================================================================================================================
// ISO 8601 Year, month and day combinations.
// ====================================================================================================================

template <>
struct json_traits<ice::date::year_month> : detail::date_traits<ice::date::year_month> {};

template <>
struct json_traits<ice::date::year_month_day> : detail::date_traits<ice::date::year_month_day> {};

// ====================================================================================================================
// Last day of a specific year and month.
// ====================================================================================================================

template <>
struct json_traits<ice::date::year_month_day_last> : detail::date_traits<ice::date::year_month_day_last> {};

// ====================================================================================================================
// Nth and last weekday of a specific year and month.
// ====================================================================================================================

template <>
struct json_traits<ice::date::year_month_weekday> : detail::date_traits<ice::date::year_month_weekday> {};

template <>
struct json_traits<ice::date::year_month_weekday_last> : detail::date_traits<ice::date::year_month_weekday_last> {};

// ====================================================================================================================
// Timepoints.
// ====================================================================================================================

template <typename Clock, typename Duration>
struct json_traits<std::chrono::time_point<Clock, Duration>> :
  detail::date_traits<std::chrono::time_point<Clock, Duration>> {};

}  // namespace ice