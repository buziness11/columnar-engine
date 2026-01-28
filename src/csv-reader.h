#pragma once

#include <cstdint>
#include <istream>
#include <vector>

// follow RFC 4180 standart
class CSVReader {
public:
    CSVReader(std::istream *input, uint8_t cnt_columns, char delim = ',', bool have_header = false);
    std::vector<std::string> GetRow();
    bool IsRead();

private:
    std::istream *input_{nullptr};
    uint8_t cnt_columns_;
    char delim_;
    bool have_header_ = false;
};
