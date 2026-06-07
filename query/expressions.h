#pragma once

#include "core/batch.h"
#include "core/column.h"
#include "core/datatype.h"
#include <cstdint>
#include <functional>
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
    BinaryCmp(std::shared_ptr<IExpression> left,
              CmpType cmp_type,
              std::shared_ptr<IExpression> right,
              const std::string& name = "");
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
    Plus,
    Minus,
    And,
    Or,
};

class BinaryFunc : public IExpression {
public:
    BinaryFunc(std::shared_ptr<IExpression> left,
               FuncType bin_func_type,
               std::shared_ptr<IExpression> right,
               const std::string& name = "");
    ~BinaryFunc() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> left_;
    FuncType bin_func_type_;
    std::shared_ptr<IExpression> right_;
    std::string name_;
};

class Like : public IExpression {
public:
    Like(std::shared_ptr<IExpression>,
         const std::string& pattern,
         bool invert = false,
         const std::string& name = "");
    ~Like() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_;
    std::string pattern_;
    bool invert_ = false;
    std::string name_;
};

enum class TimeExtractType {
    Year,
    Month,
    Day,
    Hour,
    Minute,
    Second,
};

class ExtractFromTime : public IExpression {
public:
    ExtractFromTime(std::shared_ptr<IExpression>,
                    // TimeExtractType time_extract_type,
                    const std::string& name = "");
    ~ExtractFromTime() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_;
    // TimeExtractType time_extract_type_;
    std::string name_;
    std::function<int32_t(int64_t)> extract_func_ =
        static_cast<int32_t (*)(int64_t)>(GetMinutes);
};

class TruncateTime : public IExpression {
public:
    TruncateTime(std::shared_ptr<IExpression>,
                 Trunc trunc,
                 const std::string& name = "");
    ~TruncateTime() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_;
    Trunc trunc_;
    std::string name_;
};

class Length : public IExpression {
public:
    Length(std::shared_ptr<IExpression>, const std::string& name = "");
    ~Length() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_;
    std::string name_;
};

class Case : public IExpression {
public:
    Case(std::shared_ptr<IExpression> child_predicate,
         std::shared_ptr<IExpression> child_true,
         std::shared_ptr<IExpression> child_false,
         const std::string& name = "");
    ~Case() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_predicate_;
    std::shared_ptr<IExpression> child_true_;
    std::shared_ptr<IExpression> child_false_;
    std::string name_;
};

class RegexpReplace : public IExpression {
public:
    RegexpReplace(std::shared_ptr<IExpression> child,
                  const std::string& pattern,
                  const std::string& replacement,
                  const std::string& name = "");
    ~RegexpReplace() override = default;
    Column Evaluate(const Batch&) override;
    std::string GetName() const override;

private:
    std::shared_ptr<IExpression> child_;
    std::string pattern_;
    std::string replacement_;
    std::string name_;
};
