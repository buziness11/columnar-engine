#include "batch.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <variant>
#include <vector>
#include "column.h"
#include "schema.h"
#include "types.h"

Batch::Batch(Schema&& sch, std::vector<Column>&& col)
    : schema_(std::move(sch)), data_(std::move(col)) {
    if (sch.GetCntColumns() != data_.size()) {
        DLOG(ERROR) << "schema cnt cols neq real cnt cols : "
                    << sch.GetCntColumns() << ' ' << data_.size();
        throw std::exception();
    }
}

Batch::Batch(const Schema& sch, std::vector<Column>&& col)
    : schema_(sch), data_(std::move(col)) {
    if (sch.GetCntColumns() != data_.size()) {
        DLOG(ERROR) << "schema cnt cols neq real cnt cols : "
                    << sch.GetCntColumns() << ' ' << data_.size();
        throw std::exception();
    }
}

void Batch::NewSchema(Schema schema) {
    if (schema.GetCntColumns() != schema_.GetCntColumns()) {
        DLOG(ERROR) << "Cannot put new schema in batch, wrong cnt columns";
        throw std::exception();
    }
    if (schema.GetNames().size() != 0) {
        schema_ = schema;
    }
    for (uint8_t i = 0; i < schema.GetCntColumns(); ++i) {
        data_[i].TranslateTo(schema.GetTypes()[i]);
    }
}

void Batch::NewTypes(std::vector<Types> t) {
    schema_.SetTypes(t);
    for (size_t i = 0; i < t.size(); ++i) {
        data_[i].TranslateTo(t[i]);
    }
}

const Schema& Batch::GetSchema() const {
    return schema_;
}

size_t Batch::GetCntColumns() const {
    return data_.size();
}

size_t Batch::GetColumnSize() const {
    if (data_.empty()) {
        return 0;
    } else {
        return data_[0].GetSize();
    }
}

Column& Batch::GetColumnIdx(size_t i) {
    return data_[i];
}

const Column& Batch::GetColumnIdx(size_t i) const {
    return data_[i];
}

const Column& Batch::GetColumnByName(const std::string& s) const {
    for (size_t i = 0; i < schema_.GetCntColumns(); ++i) {
        if (schema_.GetNames()[i] == s) {
            return data_[i];
        }
    }
    throw std::exception();
}

const std::vector<Column>& Batch::GetBatchData() const {
    return data_;
}