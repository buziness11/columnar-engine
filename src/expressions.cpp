#include "expressions.h"

ColumnRef::ColumnRef(const std::string& name) : column_name_(name) {
}

Column ColumnRef::Evaluate(const Batch& batch) {
    return batch.GetColumnByName(column_name_);
}
