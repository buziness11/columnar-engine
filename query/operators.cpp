#include "query/operators.h"
#include "core/batch.h"
#include "core/column.h"
#include "query/aggregate.h"
#include "io/my-format.h"
#include "core/rwconsts.h"
#include "core/schema.h"
#include "core/types.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <map>
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

std::shared_ptr<IAggregateFunc> FuncByAggregateTypeHelper(AggregateType type_,
                                                          Types out_type) {
    switch (type_) {
        case AggregateType::Count: {
            return std::make_shared<CountFunc>(CountFunc());
        }
        case AggregateType::Sum: {
            return std::make_shared<SumFunc>(out_type);
        }
        case AggregateType::Avg: {
            return std::make_shared<AvgFunc>();
        }
        case AggregateType::CountDistinct: {
            return std::make_shared<CountDistinctFunc>(out_type);
        }
        case AggregateType::Min: {
            return std::make_shared<MinFunc>(out_type);
        }
        case AggregateType::Max: {
            return std::make_shared<MaxFunc>(out_type);
        }
        default: {
            DLOG(ERROR) << "dont support such aggregation";
            throw std::exception();
        }
    }
}

std::optional<Batch> AggregateOperator::Next() {
    std::vector<std::shared_ptr<IAggregateFunc>> funcs;
    std::vector<std::shared_ptr<IAggregateState>> states;

    std::optional<Batch> nw = child_->Next();
    std::vector<Types> out_types(aggregations_.size(), Types::kString);
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        Column cl = expressions_[i]->Evaluate(*nw);
        out_types[i] = cl.GetType();
        funcs.emplace_back(
            FuncByAggregateTypeHelper(aggregations_[i], out_types[i]));
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

GroupByOperator::GroupByOperator(
    std::shared_ptr<IOperator> child, std::vector<AggregateType> aggregations,
    std::vector<std::shared_ptr<IExpression>> expressions,
    std::vector<std::shared_ptr<IExpression>> keys)
    : child_(child),
      aggregations_(aggregations),
      expressions_(expressions),
      keys_(keys) {
}

std::vector<std::string> key_encoder(
    const Batch& batch,
    const std::vector<std::shared_ptr<IExpression>>& keys_) {
    std::vector<std::string> key(batch.GetColumnSize());
    std::vector<std::vector<std::string>> str_keys(keys_.size());
    for (size_t i = 0; i < keys_.size(); ++i) {
        Column cl = keys_[i]->Evaluate(batch);
        cl.TranslateTo(Types::kString);
        str_keys[i] =
            std::move(std::get<std::vector<std::string>>(cl.GetData()));
    }
    for (size_t i = 0; i < batch.GetColumnSize(); ++i) {
        for (size_t j = 0; j < keys_.size(); ++j) {
            key[i] += str_keys[j][i];
            key[i].push_back(kStringDelimiter);
        }
        if (!key[i].empty()) {
            key[i].pop_back();
        }
    }
    return key;
}

std::vector<Column> key_decoder(const std::vector<std::string>& key,
                                size_t key_cnt) {
    std::vector<std::vector<std::string>> res(key_cnt);
    for (size_t i = 0; i < key.size(); ++i) {
        std::string nw;
        int cnt_nw = 0;
        for (size_t j = 0; j < key[i].size(); ++j) {
            if (key[i][j] == kStringDelimiter) {
                res[cnt_nw++].emplace_back(std::move(nw));
                nw.clear();
            } else {
                nw.push_back(key[i][j]);
            }
        }
        res[cnt_nw].emplace_back(std::move(nw));
    }
    std::vector<Column> res_cl;
    for (size_t i = 0; i < key_cnt; ++i) {
        res_cl.emplace_back(Column(std::move(res[i]), Types::kString));
    }
    return res_cl;
}

std::optional<Batch> GroupByOperator::Next() {
    std::vector<std::shared_ptr<IAggregateFunc>> funcs;
    std::vector<std::map<std::string, std::shared_ptr<IAggregateState>>> states(
        aggregations_.size());

    std::optional<Batch> nw = child_->Next();
    if (!nw) {
        return std::nullopt;
    }
    std::vector<std::string> str_keys = key_encoder(*nw, keys_);
    std::vector<Types> out_types(aggregations_.size(), Types::kString);
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        Column cl = expressions_[i]->Evaluate(*nw);
        out_types[i] = cl.GetType();
        funcs.emplace_back(
            FuncByAggregateTypeHelper(aggregations_[i], out_types[i]));
        for (size_t j = 0; j < str_keys.size(); ++j) {
            if (states[i].find(str_keys[j]) == states[i].end()) {
                states[i][str_keys[j]] = funcs[i]->CreateState();
            }
            funcs[i]->Update(states[i][str_keys[j]],
                             cl.GetElementByIndexAsColumn(j));
        }
    }

    while ((nw = child_->Next())) {
        str_keys = key_encoder(*nw, keys_);
        for (size_t i = 0; i < aggregations_.size(); ++i) {
            Column cl = expressions_[i]->Evaluate(*nw);
            for (size_t j = 0; j < str_keys.size(); ++j) {
                if (states[i].find(str_keys[j]) == states[i].end()) {
                    states[i][str_keys[j]] = funcs[i]->CreateState();
                }
                funcs[i]->Update(states[i][str_keys[j]],
                                 cl.GetElementByIndexAsColumn(j));
            }
        }
    }

    std::vector<Column> res_aggs(aggregations_.size());
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        DispatchColumnHelper(
            out_types[i], [&res_aggs, &out_types, &i]<Types Dst>() {
                using cpptype = EnumToCpp<Dst>::Type;
                res_aggs[i] = Column(std::vector<cpptype>(), out_types[i]);
            });
    }
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        for (auto& [a, b] : states[i]) {
            res_aggs[i].MergeWithOtherColumn(
                funcs[i]->Finalize(b, out_types[i]));
        }
    }
    std::vector<Column> res_keys;
    {
        std::vector<std::string> strs;
        for (auto& [key, _] : states[0]) {
            strs.emplace_back(key);
        }
        res_keys = key_decoder(strs, keys_.size());
    }
    std::vector<Types> key_types(keys_.size(), Types::kString);
    key_types.insert(key_types.end(), out_types.begin(), out_types.end());
    res_keys.insert(res_keys.end(), res_aggs.begin(), res_aggs.end());
    return Batch(
        Schema(std::vector<std::string>(res_keys.size(), "asd"), key_types),
        std::move(res_keys));
}

// OrderByLimitOperator::OrderByLimitOperator(std::shared_ptr<IOperator> child,
//                                            std::shared_ptr<IExpression> keys,
//                                            size_t limit)
//     : child_(std::move(child)), keys_(std::move(keys)), limit_(limit) {
// }

// std::optional<Batch> OrderByLimitOperator::Next() {
//     std::multimap<ValueType, Batch> res_map;
//     std::optional<Batch> nw = child_->Next();
//     while (nw) {
//         Column order_cl = keys_->Evaluate(*nw);
//         DispatchColumnHelper(
//             order_cl.GetType(), [&res_map, &nw, &order_cl, this]<Types Src>()
//             {
//                 using Type = typename EnumToCpp<Src>::Type;
//                 for (size_t i = 0; i < order_cl.GetSize(); ++i) {
//                     if (res_map.size() < limit_) {
//                         res_map.insert({order_cl.GetElementByIndex<Type>(i),
//                                         nw->GetRow(i)});
//                         continue;
//                     }
//                     if (order_cl.GetElementByIndex<Type>(i) <
//                         res_map.rbegin()->first) {
//                         res_map.erase(res_map.rbegin()->first);
//                         res_map.insert({order_cl.GetElementByIndex<Type>(i),
//                                         nw->GetRow(i)});
//                     }
//                 }
//             });
//         nw = child_->Next();
//     }

//     if (res_map.empty()) {
//         return std::nullopt;
//     }

//     auto it = res_map.begin();
//     Batch res = std::move(it->second);
//     ++it;
//     for (; it != res_map.end(); ++it) {
//         res.MergeWithOtherBatch(std::move(it->second));
//     }
//     return res;
// }
