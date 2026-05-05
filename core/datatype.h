#pragma once
#include <cstdint>
#include <string>

int32_t DaysCount(const std::string& yyyy_mm_dd);
int64_t SecondsCount(const std::string& yyyy_mm_dd_hh_mm_ss);

std::string GetYyyyMmDd(int32_t days);
std::string GetYyyyMmDdHhMmSs(int64_t seconds);