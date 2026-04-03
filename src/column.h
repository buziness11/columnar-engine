#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <variant>
#include "types.h"

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

using ColumnType = std::variant<std::vector<int16_t>, std::vector<int32_t>,
                                std::vector<int64_t>, std::vector<std::string>>;

template <typename F>
void DispatchColumnHelper(Types from, F&& f) {
    switch (from) {
        case Types::kInt16_t:
            f.template operator()<Types::kInt16_t>();
            break;

        case Types::kInt32_t:
            f.template operator()<Types::kInt32_t>();
            break;

        case Types::kInt64_t:
            f.template operator()<Types::kInt64_t>();
            break;

        case Types::kString:
            f.template operator()<Types::kString>();
            break;

        case Types::kDate:
            f.template operator()<Types::kDate>();
            break;

        case Types::kTimestamp:
            f.template operator()<Types::kTimestamp>();
            break;
    }
}

class Column {
public:
    Column() = default;
    Column(const Column&) = default;
    Column(Column&&) = default;
    Column& operator=(const Column&) = default;
    Column& operator=(Column&&) = default;
    ~Column() = default;

    template <typename T>
    Column(const std::vector<T>& vec, Types tp) : data_(vec), type_(tp) {
    }
    template <typename T>
    Column(std::vector<T>&& vec, Types tp) : data_(vec), type_(tp) {
    }

    template <typename T>
    T GetElementByIndex(size_t idx) {
        return std::get<std::vector<T>>(data_)[idx];
    }

    ColumnType& GetData() &;
    ColumnType&& GetData() &&;
    const ColumnType& GetData() const;
    void TranslateTo(Types);
    size_t GetSize() const;
    Types GetType() const;

    void PrintCol();

private:
    ColumnType data_;
    Types type_;
};

int64_t WriteColToBzn(std::fstream* file, Column&& c);

// Column in half-open [file->tellg, end)
Column ReadColFromBzn(std::fstream* file, Types t, int64_t end);