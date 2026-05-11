#pragma once
#include <cstdint>
#include <string>

int32_t inline GetSeconds(const std::string& yyyy_mm_dd_hh_mm_ss);
int32_t inline GetMinutes(const std::string& yyyy_mm_dd_hh_mm_ss);
int32_t inline GetHours(const std::string& yyyy_mm_dd_hh_mm_ss);
int32_t inline GetDays(const std::string& yyyy_mm_dd_hh_mm_ss);
int32_t inline GetMonths(const std::string& yyyy_mm_dd_hh_mm_ss);
int32_t inline GetYears(const std::string& yyyy_mm_dd_hh_mm_ss);

int32_t inline GetSeconds(int64_t seconds);
int32_t inline GetMinutes(int64_t seconds);
int32_t inline GetHours(int64_t seconds);
int32_t inline GetDays(int64_t seconds);
int32_t inline GetMonths(int64_t seconds);
int32_t inline GetYears(int64_t seconds);

int32_t DaysCount(const std::string& yyyy_mm_dd);
int64_t SecondsCount(const std::string& yyyy_mm_dd_hh_mm_ss);

std::string GetYyyyMmDd(int32_t days);
std::string GetYyyyMmDdHhMmSs(int64_t seconds);
