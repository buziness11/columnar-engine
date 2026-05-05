#pragma once

#include "core/batch.h"
#include "core/column.h"
#include "query/expressions.h"
#include "io/my-format.h"
#include <memory>
#include <optional>
#include <string>

class IOperator {
public:
    virtual ~IOperator() = default;
    virtual std::optional<Batch> Next() = 0;
};

class ScanOperator : public IOperator {
public:
    ScanOperator(BZNReader&&, std::vector<std::string>&&);
    std::optional<Batch> Next() override;
    ~ScanOperator() override = default;

private:
    BZNReader bzn_reader_;
    std::vector<std::string> column_names_;
};

class FilterOperator : public IOperator {
public:
    FilterOperator(std::shared_ptr<IOperator> child,
                   std::shared_ptr<IExpression>);
    std::optional<Batch> Next() override;
    ~FilterOperator() override = default;

private:
    std::shared_ptr<IOperator> child_;
    std::shared_ptr<IExpression> predicate_;
};

enum class AggregateType { Sum, Count, CountDistinct, Min, Max, Avg };

class AggregateOperator : public IOperator {
public:
    AggregateOperator(std::shared_ptr<IOperator> child,
                      std::vector<AggregateType> aggregations,
                      std::vector<std::shared_ptr<IExpression>> expressions_);
    std::optional<Batch> Next() override;

private:
    std::shared_ptr<IOperator> child_;
    std::vector<AggregateType> aggregations_;
    std::vector<std::shared_ptr<IExpression>> expressions_;
};

class GroupByOperator : public IOperator {
public:
    GroupByOperator(std::shared_ptr<IOperator> child,
                    std::vector<AggregateType> aggregations,
                    std::vector<std::shared_ptr<IExpression>> expressions,
                    std::vector<std::shared_ptr<IExpression>> keys);
    std::optional<Batch> Next() override;

private:
    std::shared_ptr<IOperator> child_;
    std::vector<AggregateType> aggregations_;
    std::vector<std::shared_ptr<IExpression>> expressions_;
    std::vector<std::shared_ptr<IExpression>> keys_;
};

// class OrderByLimitOperator : public IOperator {
// public:
//     OrderByLimitOperator(std::shared_ptr<IOperator> child,
//                          std::shared_ptr<IExpression> keys, size_t limit);
//     std::optional<Batch> Next() override;

// private:
//     std::shared_ptr<IOperator> child_;
//     std::shared_ptr<IExpression> keys_;
//     size_t limit_;
// };
