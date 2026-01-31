#pragma once

#include "batch.h"
#include "column.h"
#include "schema.h"
#include <vector>
#include <cstdint>
#include <cstddef>
#include <fstream>

const char kMetaDelimiter = '\x1E';
const char kStringDelimiter = '\x1F';

int64_t WriteCol(std::fstream*, Column&&);

class BZNReader {
public:
    BZNReader(std::fstream* file);
    Batch Read();
    bool IsReaded();

private:
    void GetMetaOffset();
    void BuildSchema();
    std::vector<int64_t> GetMetaBatchOffset();
    std::vector<std::string> GetMetaString();
    std::fstream* ma_format_;
    std::vector<int64_t> offsets_;
    Schema schema_;
    size_t cur_batch_;
};

class BZNWriter {
public:
    BZNWriter(Schema, std::fstream*);
    void Write(Batch);
    void WriteMetaInfo();

private:
    Schema schema_;
    std::fstream* ma_format_;
    std::vector<int64_t> offsets_;
    size_t batch_row_size_;
    bool locked_;
};
