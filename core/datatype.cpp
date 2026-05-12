#include "core/datatype.h"
#include "core/rwconsts.h"
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <exception>
#include <format>
#include <glog/logging.h>
#include <string>

// В нашей таске нам неважно работать с чужими форматами
// Нам важно поддерживать следующие операции
// data_1 </=/>/<=/>=/+/- data_2
// Поэтому поддерживаются следующие суждения
// 1. В месяце 32 дня
// 2. В году 512 дней

int32_t inline GetSeconds(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int32_t second{};
    std::from_chars(yyyy_mm_dd_hh_mm_ss.data() + 17,
                    yyyy_mm_dd_hh_mm_ss.data() + 19,
                    second);
    return second;
}

int32_t inline GetMinutes(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int32_t minute{};
    std::from_chars(yyyy_mm_dd_hh_mm_ss.data() + 14,
                    yyyy_mm_dd_hh_mm_ss.data() + 16,
                    minute);
    return minute;
}

int32_t inline GetHours(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int32_t hour{};
    std::from_chars(yyyy_mm_dd_hh_mm_ss.data() + 11,
                    yyyy_mm_dd_hh_mm_ss.data() + 13,
                    hour);
    return hour;
}

int32_t inline GetDays(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int32_t day{};
    std::from_chars(yyyy_mm_dd_hh_mm_ss.data() + 8,
                    yyyy_mm_dd_hh_mm_ss.data() + 10,
                    day);
    return day;
}

int32_t inline GetMonths(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int32_t month{};
    std::from_chars(yyyy_mm_dd_hh_mm_ss.data() + 5,
                    yyyy_mm_dd_hh_mm_ss.data() + 7,
                    month);
    return month;
}
int32_t inline GetYears(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int32_t year{};
    std::from_chars(yyyy_mm_dd_hh_mm_ss.data(),
                    yyyy_mm_dd_hh_mm_ss.data() + 4,
                    year);
    return year;
}

int32_t inline GetSeconds(int64_t seconds) {
    return seconds % 60;
}
int32_t inline GetMinutes(int64_t seconds) {
    return (seconds / kSecondsPerMinute) % 60;
}
int32_t inline GetHours(int64_t seconds) {
    return (seconds / kSecondsPerHour) % 24;
}
int32_t inline GetDays(int64_t seconds) {
    return (seconds / kSecondsPerDay) % 32 + 1;
}
int32_t inline GetMonths(int64_t seconds) {
    return (seconds / kSecondsPerDay) / 32 + 1;
}
int32_t inline GetYears(int64_t seconds) {
    return (seconds / kSecondsPerDay) / 512 + 1970;
}

int32_t DaysCount(const std::string& yyyy_mm_dd) {
    int32_t year = GetYears(yyyy_mm_dd);
    year -= 1970;

    int32_t month = GetMonths(yyyy_mm_dd);
    month--;

    int32_t day = GetDays(yyyy_mm_dd);
    day--;
    day += year * 512;
    day += month * 32;
    return day;
}

int64_t SecondsCount(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int64_t hour = GetHours(yyyy_mm_dd_hh_mm_ss);
    int64_t minute = GetMinutes(yyyy_mm_dd_hh_mm_ss);
    int64_t second = GetSeconds(yyyy_mm_dd_hh_mm_ss);
    int64_t day = DaysCount(yyyy_mm_dd_hh_mm_ss);
    return day * kSecondsPerDay + hour * kSecondsPerHour +
           minute * kSecondsPerMinute + second;
}

std::string GetYyyyMmDd(int32_t days) {
    int32_t year = 1970 + days / 512;
    days %= 512;
    int32_t month = 1 + days / 32;
    days %= 32;
    days++;
    return std::format("{:04d}-{:02d}-{:02d}", year, month, days);
}

std::string GetYyyyMmDdHhMmSs(int64_t seconds) {
    int32_t days = seconds / kSecondsPerDay;
    seconds = seconds % kSecondsPerDay;
    return GetYyyyMmDd(days) +
           std::format(" {:02d}:{:02d}:{:02d}",
                       seconds / kSecondsPerHour,
                       (seconds % kSecondsPerHour) / kSecondsPerMinute,
                       seconds % kSecondsPerMinute);
}