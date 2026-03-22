#include "datatype.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <format>
#include <string>
#include <glog/logging.h>
#include "rwconsts.h"
inline bool IsLeap(int32_t year) {
    return (year % 400 == 0) || (year % 100 != 0 and year % 4 == 0);
}

int32_t DaysCount(const std::string& yyyy_mm_dd) {
    int64_t year = std::stoll(
        std::move(std::string(yyyy_mm_dd.begin(), yyyy_mm_dd.begin() + 4)));
    year -= 1970;
    int64_t month = std::stoll(
        std::move(std::string(yyyy_mm_dd.begin() + 5, yyyy_mm_dd.begin() + 7)));
    month -= 1;
    int64_t day = std::stoll(std::move(
        std::string(yyyy_mm_dd.begin() + 8, yyyy_mm_dd.begin() + 10)));
    day -= 1;
    day += kYearPrefDays[year];
    if (IsLeap(year)) {
        day += kMonthPrefDaysLeap[month];
    } else {
        day += kMonthPrefDays[month];
    }
    return day;
}

int64_t SecondsCount(const std::string& yyyy_mm_dd_hh_mm_ss) {
    int64_t hour = std::stoll(std::move(std::string(
        yyyy_mm_dd_hh_mm_ss.begin() + 11, yyyy_mm_dd_hh_mm_ss.begin() + 13)));
    int64_t minute = std::stoll(std::move(std::string(
        yyyy_mm_dd_hh_mm_ss.begin() + 14, yyyy_mm_dd_hh_mm_ss.begin() + 16)));
    int64_t second = std::stoll(std::move(std::string(
        yyyy_mm_dd_hh_mm_ss.begin() + 17, yyyy_mm_dd_hh_mm_ss.begin() + 19)));
    int64_t day = DaysCount(yyyy_mm_dd_hh_mm_ss);
    return day * kSecondsPerDay + hour * kSecondsPerHour +
           minute * kSecondsPerMinute + second;
}

std::string GetYyyyMmDd(int32_t days) {
    auto it_year =
        std::upper_bound(kYearPrefDays.begin(), kYearPrefDays.end(), days);
    if (it_year == kYearPrefDays.end()) {
        DLOG(ERROR) << "bad day";
        throw std::exception();
    }
    days -= *(--it_year);
    int y_diff = it_year - kYearPrefDays.begin();
    int year = 1970 + y_diff;

    std::array<const int32_t, 13>::iterator it_month;
    std::array<const int32_t, 13>::iterator beg;
    if (IsLeap(year)) {
        beg = kMonthPrefDaysLeap.begin();
        it_month = std::upper_bound(kMonthPrefDaysLeap.begin(),
                                    kMonthPrefDaysLeap.end(), days);
    } else {
        beg = kMonthPrefDays.begin();
        it_month = std::upper_bound(kMonthPrefDays.begin(),
                                    kMonthPrefDays.end(), days);
    }
    int month = it_month - beg;
    days -= *(--it_month);
    days += 1;
    return std::format("{:04d}-{:02d}-{:02d}", year, month, days);
}

std::string GetYyyyMmDdHhMmSs(int64_t seconds) {
    int32_t days = seconds / kSecondsPerDay;
    seconds = seconds % kSecondsPerDay;
    return GetYyyyMmDd(days) +
           std::format(" {:02d}:{:02d}:{:02d}", seconds / kSecondsPerHour,
                       (seconds % kSecondsPerHour) / kSecondsPerMinute,
                       seconds % kSecondsPerMinute);
}