#include "operators.h"
#include "batch.h"
#include "column.h"
#include "aggregate.h"
#include "my-format.h"
#include "schema.h"
#include "types.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <variant>

ScanOperator::ScanOperator(BZNReader&& bzn_reader,
                           std::vector<std::string>&& column_names)
    : bzn_reader_(bzn_reader), column_names_(column_names) {
}

std::optional<Batch> ScanOperator::Next() {
    if (bzn_reader_.IsReaded()) {
        return std::nullopt;
    }
    Batch getted = bzn_reader_.Read(column_names_);
    return getted;
}

FilterOperator::FilterOperator(std::shared_ptr<IOperator> child,
                               std::shared_ptr<IExpression> op)
    : child_(child), predicate_(op) {
}

std::optional<Batch> FilterOperator::Next() {
    std::optional<Batch> new_batch = child_->Next();
    if (!new_batch) {
        return std::nullopt;
    }
    Column mask = predicate_->Evaluate(*new_batch);
    std::vector<Column> res;
    for (const Column& c : new_batch->GetBatchData()) {
        std::visit(Overloaded{[&mask, &res, &c](auto&& vec) {
                       if (mask.GetSize() != vec.size()) {
                           DLOG(ERROR) << "Mask size neq column size";
                           throw std::exception();
                       }
                       std::decay_t<decltype(vec)> nw;
                       for (size_t i = 0; i < vec.size(); ++i) {
                           if (mask.GetElementByIndex<bool>(i)) {
                               nw.emplace_back(vec[i]);
                           }
                       }
                       DLOG(INFO) << nw.size() << ' ' << vec.size();
                       res.emplace_back(Column(std::move(nw), c.GetType()));
                   }},
                   c.GetData());
    }
    return Batch(new_batch->GetSchema(), std::move(res));
}

AggregateOperator::AggregateOperator(
    std::shared_ptr<IOperator> child, std::vector<AggregateType> aggregations,
    std::vector<std::shared_ptr<IExpression>> expressions)
    : child_(child), aggregations_(aggregations), expressions_(expressions) {
    if (aggregations_.size() != expressions.size()) {
        DLOG(ERROR) << "Wrong agg/expr size";
        throw std::exception();
    }
}

std::optional<Batch> AggregateOperator::Next() {
    std::vector<std::shared_ptr<IAggregateFunc>> funcs;
    std::vector<std::shared_ptr<IAggregateState>> states;
    for (size_t i = 0; i < aggregations_.size(); ++i) {
    }

    std::optional<Batch> nw = child_->Next();
    std::vector<Types> out_types(aggregations_.size(), Types::kString);
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        Column cl = expressions_[i]->Evaluate(*nw);
        out_types[i] = cl.GetType();

        switch (aggregations_[i]) {
            case AggregateType::Count: {
                funcs.emplace_back(std::make_shared<CountFunc>(CountFunc()));
                break;
            }
            case AggregateType::Sum: {
                funcs.emplace_back(std::make_shared<SumFunc>(out_types[i]));
                break;
            }
            case AggregateType::Avg: {
                funcs.emplace_back(std::make_shared<AvgFunc>());
                break;
            }
            case AggregateType::CountDistinct: {
                funcs.emplace_back(
                    std::make_shared<CountDistinctFunc>(out_types[i]));
                break;
            }
            case AggregateType::Min: {
                funcs.emplace_back(std::make_shared<MinFunc>(out_types[i]));
                break;
            }
            case AggregateType::Max: {
                funcs.emplace_back(std::make_shared<MaxFunc>(out_types[i]));
                break;
            }
            default: {
                DLOG(ERROR) << "dont support such aggregation";
                throw std::exception();
            }
        }
        states.emplace_back(funcs.back()->CreateState());
        funcs[i]->Update(states[i], cl);
    }

    nw = child_->Next();
    while (nw) {
        for (size_t i = 0; i < aggregations_.size(); ++i) {
            Column cl = expressions_[i]->Evaluate(*nw);

            funcs[i]->Update(states[i], cl);
        }
        nw = child_->Next();
    }

    std::vector<Column> res;
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        res.emplace_back(funcs[i]->Finalize(states[i], out_types[i]));
    }
    return Batch(Schema(std::vector<std::string>(res.size(), "asd"), out_types),
                 std::move(res));
}
