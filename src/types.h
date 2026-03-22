#pragma once

#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <glog/logging.h>
#include "datatype.h"

enum class Types { kInt16_t, kInt32_t, kInt64_t, kString, kDate, kTimestamp };

std::string TypeToString(Types t);

Types StringToType(const std::string& s);
Types StringToType(std::string&& s);

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

template <Types T>
concept NumericType =
    (T == Types::kInt16_t) || (T == Types::kInt32_t) || (T == Types::kInt64_t);

template <Types T>
concept StringType = (T == Types::kString);

template <Types T, Types U, typename X>
    requires(T == U)
X TranslateTtoU(X a) {
    return a;
}

template <Types T, Types U, typename X>
    requires NumericType<T> && StringType<U>
std::string TranslateTtoU(X a) {
    return std::to_string(a);
}

template <Types T, Types U>
    requires StringType<T> && NumericType<U>
auto TranslateTtoU(std::string s) {
    return std::stoll(s);  // poh
}

template <Types T, Types U, typename X>
    requires NumericType<T> && NumericType<U>
auto TranslateTtoU(X a) {
    return a;  // poh
}

template <Types T, Types U>
    requires(T == Types::kDate) && (U == Types::kString)
std::string TranslateTtoU(int32_t days_from_1970_01_01) {
    return GetYyyyMmDd(days_from_1970_01_01);
}

template <Types T, Types U>
    requires(U == Types::kDate) && (T == Types::kString)
int32_t TranslateTtoU(const std::string& yyyy_mm_dd) {
    return DaysCount(yyyy_mm_dd);
}

template <Types T, Types U>
    requires(T == Types::kTimestamp) && (U == Types::kString)
std::string TranslateTtoU(int64_t seconds_from_1970_01_01) {
    return GetYyyyMmDdHhMmSs(seconds_from_1970_01_01);
}

template <Types T, Types U>
    requires(U == Types::kTimestamp) && (T == Types::kString)
int64_t TranslateTtoU(const std::string& yyyy_mm_dd_hh_mm_ss) {
    return SecondsCount(yyyy_mm_dd_hh_mm_ss);
}

template <Types T>
struct EnumToCpp;
template <>
struct EnumToCpp<Types::kInt16_t> {
    using Type = int16_t;
};
template <>
struct EnumToCpp<Types::kInt32_t> {
    using Type = int32_t;
};
template <>
struct EnumToCpp<Types::kInt64_t> {
    using Type = int64_t;
};
template <>
struct EnumToCpp<Types::kString> {
    using Type = std::string;
};
template <>
struct EnumToCpp<Types::kDate> {
    using Type = int32_t;
};
template <>
struct EnumToCpp<Types::kTimestamp> {
    using Type = int64_t;
};