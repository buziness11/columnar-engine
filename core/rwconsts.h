#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
const char kMetaDelimiter = '\x1E';
const char kStringDelimiter = '\x1F';
const size_t kBatchRowSize = 500'000;
const size_t kMaxStringLenghtCsvSize = 1ull << 20;

// support 1970 - 2040
const std::array<int32_t, 70> kYearPrefDays{
    0,     365,   730,   1096,  1461,  1826,  2191,  2557,  2922,  3287,
    3652,  4018,  4383,  4748,  5113,  5479,  5844,  6209,  6574,  6940,
    7305,  7670,  8035,  8401,  8766,  9131,  9496,  9862,  10227, 10592,
    10957, 11323, 11688, 12053, 12418, 12784, 13149, 13514, 13879, 14245,
    14610, 14975, 15340, 15706, 16071, 16436, 16801, 17167, 17532, 17897,
    18262, 18628, 18993, 19358, 19723, 20089, 20454, 20819, 21184, 21550,
    21915, 22280, 22645, 23011, 23376, 23741, 24106, 24472, 24837, 25202};

const std::array<int32_t, 13> kMonthPrefDays{0,   31,  59,  90,  120, 151, 181,
                                             212, 243, 273, 304, 334, 365};
const std::array<int32_t, 13> kMonthPrefDaysLeap{
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

const int64_t kSecondsPerMinute = 60;
const int64_t kSecondsPerHour = 60 * kSecondsPerMinute;
const int64_t kSecondsPerDay = 24 * kSecondsPerHour;