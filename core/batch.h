#pragma once

#include <cstddef>
#include "core/column.h"
#include "core/schema.h"

class Batch {
public:
    Batch() = default;
    Batch(Schema&& schema, std::vector<Column>&& data);
    Batch(const Schema& sch, std::vector<Column>&& col);
    void NewSchema(Schema schema);
    void NewTypes(std::vector<Types>);
    const Schema& GetSchema() const;
    size_t GetCntColumns() const;
    size_t GetColumnSize() const;
    Column& GetColumnIdx(size_t i);
    Batch GetRow(size_t i);
    void MergeWithOtherBatch(Batch&& other);
    const Column& GetColumnIdx(size_t i) const;
    const Column& GetColumnByName(const std::string&) const;
    const std::vector<Column>& GetBatchData() const;

private:
    Schema schema_;
    std::vector<Column> data_;
};
