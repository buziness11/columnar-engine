#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <vector>
#include "batch.h"

// follow RFC 4180 standart if crlf
class CSVReader {
public:
    CSVReader(std::fstream *input, size_t cnt_columns, char delim = ',',
              bool lf = true, bool have_header = false);
    std::vector<std::string> GetRow();
    Batch GetBatch(size_t batch_row_size = kBatchRowSize);
    bool IsReaded();

private:
    std::istream *input_{nullptr};
    size_t cnt_columns_;
    char delim_;
    bool lf_;
};

class CSVWriter {
public:
    CSVWriter(std::fstream *output, bool lf = true);
    void WriteBatch(Batch, char delim = ',');

private:
    std::fstream *out_;
    bool lf_;
};