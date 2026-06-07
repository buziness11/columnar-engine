#pragma once

#include <charconv>
#include <cstdint>
#include <string>
#include "core/rwconsts.h"

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

int32_t DaysCount(const std::string& yyyy_mm_dd);
int64_t SecondsCount(const std::string& yyyy_mm_dd_hh_mm_ss);

std::string GetYyyyMmDd(int32_t days);
std::string GetYyyyMmDdHhMmSs(int64_t seconds);

enum class Trunc { KSeconds, KMinutes, KHours, KDays, KMonths, KYears };

int64_t TruncateTimestamp(int64_t seconds, Trunc trunc);
