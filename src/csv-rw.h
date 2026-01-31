#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <vector>
#include "batch.h"

// follow RFC 4180 standart
class CSVReader {
public:
    CSVReader(std::fstream *input, uint8_t cnt_columns, char delim = ',',
              bool have_header = false);
    std::vector<std::string> GetRow();
    Batch GetBatch(size_t batch_row_size = kBatchRowSize);
    bool IsReaded();

private:
    std::istream *input_{nullptr};
    uint8_t cnt_columns_;
    char delim_;
};

class CSVWriter {
public:
    CSVWriter(std::fstream *output);
    void WriteBatch(Batch);

private:
    std::fstream *out_;
};