#pragma once

#include "core/batch.h"
#include "core/column.h"
#include <memory>
#include <string>

class IExpression {
public:
    virtual ~IExpression() = default;
    virtual Column Evaluate(const Batch&) = 0;
    virtual std::string GetName() const = 0;
};

class ColumnRef : public IExpression {
public:
    ColumnRef(const std::string& name = "");
    ~ColumnRef() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::string name_;
};

template <typename T>
class Literal : public IExpression {
public:
    Literal(T val, Types col_type, const std::string& name = "")
        : value_(val), col_type_(col_type), name_(name) {
    }
    ~Literal() = default;
    Column Evaluate(const Batch& batch) override {
        return Column(std::vector<T>(batch.GetColumnSize(), value_), col_type_);
    }
    std::string GetName() const override {
        return name_;
    }

private:
    T value_;
    Types col_type_;
    std::string name_;
};

enum class CmpType { L, Leq, Eq, G, Geq, Neq };

class BinaryCmp : public IExpression {
public:
    BinaryCmp(std::shared_ptr<IExpression> left, CmpType cmp_type,
              std::shared_ptr<IExpression> right, const std::string& name = "");
    ~BinaryCmp() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> left_;
    CmpType cmp_type_;
    std::shared_ptr<IExpression> right_;
    std::string name_;
};

enum class FuncType {
    Sum,
};

class BinaryFunc : public IExpression {
public:
    ~BinaryFunc() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> left_;
    FuncType bin_func_;
    std::shared_ptr<IExpression> right_;
    std::string name_;
};

class Like : public IExpression {
public:
    Like(std::shared_ptr<IExpression>, const std::string& pattern,
         const std::string& name = "");
    ~Like() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_;
    std::string pattern_;
    std::string name_;
};
