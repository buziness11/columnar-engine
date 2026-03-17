#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <istream>
#include "types.h"

class Schema {
public:
    Schema(const Schema&) = default;
    Schema(Schema&&) = default;
    Schema& operator=(const Schema&) = default;
    Schema& operator=(Schema&&) = default;
    ~Schema() = default;

    Schema();
    Schema(std::fstream* schema, char schema_delim = ',', bool lf = true);
    Schema(std::vector<std::string> names, std::vector<Types> types);

    const std::vector<std::string>& GetNames() const;
    const std::vector<Types>& GetTypes() const;
    size_t GetCntColumns() const;
    Types GetType(size_t idx) const;

    // void SetNames(std::vector<std::string>);
    void SetTypes(const std::vector<Types>&);

private:
    size_t cnt_columns_;
    std::vector<std::string> column_names_;
    std::vector<Types> column_types_;
};