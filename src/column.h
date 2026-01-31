#pragma once

#include <cstddef>
#include <cstdint>
#include <exception>
#include <variant>
#include "types.h"

using ColumnType =
    std::variant<std::vector<int64_t>, std::vector<std::string>>;

class Column {
public:
    Column() = default;
    Column(const Column&) = default;
    Column(Column&&) = default;
    Column& operator=(const Column&) = default;
    Column& operator=(Column&&) = default;
    ~Column() = default;

    template <typename T>
    Column(const std::vector<T>& vec) {
        data_ = vec;
    }
    template <typename T>
    Column(std::vector<T>&& vec) {
        data_ = std::move(vec);
    }

    ColumnType& GetData();
    void TranslateTo(Types);
    size_t GetSize();

    void PrintCol();

private:
    ColumnType data_;
};

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;
