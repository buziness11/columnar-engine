#pragma once

#include "batch.h"
#include "column.h"
#include <memory>

class IExpression {
public:
    virtual ~IExpression() = default;
    virtual Column Evaluate(const Batch&) = 0;
};

class ColumnRef : public IExpression {
public:
    ColumnRef(const std::string& name);
    ~ColumnRef() override = default;
    Column Evaluate(const Batch&) override;

private:
    std::string column_name_;
};

template <typename T>
class Literal : public IExpression {
public:
    Literal(T val, Types col_type) : value_(val), col_type_(col_type) {
    }
    ~Literal() = default;
    Column Evaluate(const Batch& batch) override {
        return Column(std::vector<T>(batch.GetColumnSize(), value_), col_type_);
    }

private:
    T value_;
    Types col_type_;
};

enum class CmpType { L, Leq, Eq, G, Geq, Neq };

class BinaryCmp : public IExpression {
public:
    BinaryCmp(std::shared_ptr<IExpression> left, CmpType cmp_type,
              std::shared_ptr<IExpression> right);
    ~BinaryCmp() override = default;
    Column Evaluate(const Batch&) override;

private:
    std::shared_ptr<IExpression> left_;
    CmpType cmp_type_;
    std::shared_ptr<IExpression> right_;
};

enum class FuncType {
    Sum,
};

class BinaryFunc : public IExpression {
public:
    ~BinaryFunc() override = default;
    Column Evaluate(const Batch&) override;

private:
    std::shared_ptr<IExpression> left_;
    std::shared_ptr<IExpression> right_;
    FuncType bin_func_;
};
