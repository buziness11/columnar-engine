#pragma once

#include <cstddef>
#include <cstdint>
const char kMetaDelimiter = '\x1E';
const char kStringDelimiter = '\x1F';
const size_t kBatchRowSize = 500'000;
const size_t kMaxStringLenghtCsvSize = 1ull << 20;

const int64_t kSecondsPerMinute = 60;
const int64_t kSecondsPerHour = 60 * kSecondsPerMinute;
const int64_t kSecondsPerDay = 24 * kSecondsPerHour;