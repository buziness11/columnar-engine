#pragma once

#include <charconv>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <chrono>
#include <sstream>
#include <glog/logging.h>
#include "core/datatype.h"

enum class Types {
    kInt16_t,
    kInt32_t,
    kInt64_t,
    kString,
    kDate,
    kTimestamp,
    kBool,
    kDouble,
    kLongDouble
};

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
template <>
struct EnumToCpp<Types::kDouble> {
    using Type = double;
};
template <>
struct EnumToCpp<Types::kLongDouble> {
    using Type = long double;
};
template <>
struct EnumToCpp<Types::kBool> {
    using Type = bool;
};

template <typename T>
struct CppToEnum;
template <>
struct CppToEnum<int16_t> {
    static constexpr Types value = Types::kInt16_t;
};
template <>
struct CppToEnum<int32_t> {
    static constexpr Types value = Types::kInt32_t;
};
template <>
struct CppToEnum<int64_t> {
    static constexpr Types value = Types::kInt64_t;
};
template <>
struct CppToEnum<std::string> {
    static constexpr Types value = Types::kString;
};
template <>
struct CppToEnum<double> {
    static constexpr Types value = Types::kDouble;
};
template <>
struct CppToEnum<long double> {
    static constexpr Types value = Types::kLongDouble;
};
template <>
struct CppToEnum<bool> {
    static constexpr Types value = Types::kBool;
};

using ColumnType = std::variant<std::vector<int16_t>,
                                std::vector<int32_t>,
                                std::vector<int64_t>,
                                std::vector<std::string>,
                                std::vector<double>,
                                std::vector<long double>,
                                std::vector<bool>>;

using ValueType = std::
    variant<int16_t, int32_t, int64_t, std::string, double, long double, bool>;

std::string TypeToString(Types t);

Types StringToType(const std::string& s);
Types StringToType(std::string&& s);

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

template <Types T>
concept NumericType = (T == Types::kInt16_t) || (T == Types::kInt32_t) ||
                      (T == Types::kInt64_t) || (T == Types::kDouble) ||
                      (T == Types::kLongDouble);

template <Types T>
concept StringType = (T == Types::kString);

template <Types T, Types U, typename X>
requires(T == U) X TranslateTtoU(X a) {
    return a;
}

template <Types T, Types U, typename X>
requires NumericType<T>&& StringType<U> std::string TranslateTtoU(X a) {
    return std::to_string(a);
}

template <Types T, Types U>
    requires StringType<T>&& NumericType<U> &&
    (U != Types::kLongDouble) auto TranslateTtoU(const std::string& s) {
    using ToType = typename EnumToCpp<U>::Type;
    ToType res;
    std::from_chars(s.data(), s.data() + s.size(), res);
    return res;
}

template <Types T, Types U>
    requires StringType<T>&& NumericType<U> &&
    (U == Types::kLongDouble) auto TranslateTtoU(const std::string& s) {
    return std::stold(s);
}

template <Types T, Types U, typename X>
requires NumericType<T>&& NumericType<U>&& std::is_arithmetic_v<X> auto
TranslateTtoU(X a) {
    using ToType = typename EnumToCpp<U>::Type;
    return static_cast<ToType>(a);
}

template <Types T, Types U>
    requires(T == Types::kDate) && (U == Types::kString) std::string
    TranslateTtoU(int32_t days_from_1970_01_01) {
    return GetYyyyMmDd(days_from_1970_01_01);
}

template <Types T, Types U>
    requires(T == Types::kString) && (U == Types::kDate) int32_t
    TranslateTtoU(const std::string& yyyy_mm_dd) {
    return DaysCount(yyyy_mm_dd);
}

template <Types T, Types U>
    requires(T == Types::kTimestamp) && (U == Types::kString) std::string
    TranslateTtoU(int64_t seconds_from_1970_01_01) {
    return GetYyyyMmDdHhMmSs(seconds_from_1970_01_01);
}

template <Types T, Types U>
    requires(T == Types::kString) && (U == Types::kTimestamp) int64_t
    TranslateTtoU(const std::string& yyyy_mm_dd_hh_mm_ss) {
    return SecondsCount(yyyy_mm_dd_hh_mm_ss);
}
