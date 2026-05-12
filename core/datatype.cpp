#include "core/datatype.h"
#include <algorithm>
#include <array>
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

int32_t DaysCount(const std::string& yyyy_mm_dd) {
    int32_t year;
    std::from_chars(yyyy_mm_dd.data(), yyyy_mm_dd.data() + 4, year);
    year -= 1970;

    int32_t month;
    std::from_chars(yyyy_mm_dd.data() + 5, yyyy_mm_dd.data() + 7, month);
    month--;

    int32_t day;
    std::from_chars(yyyy_mm_dd.data() + 8, yyyy_mm_dd.data() + 10, day);
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
