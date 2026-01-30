#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <vector>
#include "batch.h"

// follow RFC 4180 standart
class CSVReader {
public:
    CSVReader(std::istream *input, uint8_t cnt_columns, char delim = ',',
              bool have_header = false);
    std::vector<std::string> GetRow();
    Batch GetButch(size_t butch_row_size = kBatchRowSize);
    bool IsRead();

private:
    std::istream *input_{nullptr};
    uint8_t cnt_columns_;
    char delim_;
    bool have_header_ = false;
};
