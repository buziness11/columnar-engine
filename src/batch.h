#pragma once

#include <cstddef>
#include "column.h"
#include "schema.h"

class Batch {
public:
    Batch() = default;
    Batch(Batch&&) = default;
    Batch(Schema&& schema, std::vector<Column>&& data);
    Batch(const Schema& sch, std::vector<Column>&& col);
    void NewSchema(Schema schema);
    void NewTypes(std::vector<Types>);
    const Schema& GetSchema() const;
    size_t GetCntColumns();
    size_t GetColumnSize();
    Column& GetColumnIdx(size_t i);

private:
    Schema schema_;
    std::vector<Column> data_;
};
