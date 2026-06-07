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
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

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

std::string AggregateTypeToString(AggregateType type) {
    switch (type) {
        case AggregateType::Sum: {
            return "sum";
        }
        case AggregateType::Count: {
            return "count";
        }
        case AggregateType::Avg: {
            return "avg";
        }
        case AggregateType::CountDistinct: {
            return "count_distinct";
        }
        case AggregateType::Min: {
            return "min";
        }
        case AggregateType::Max: {
            return "max";
        }
    }
    DLOG(ERROR) << "Wrong aggregate type";
    throw std::exception();
}

AggregateType StringToAggregateType(const std::string& str) {
    if (str == "sum") {
        return AggregateType::Sum;
    } else if (str == "count") {
        return AggregateType::Count;
    } else if (str == "avg") {
        return AggregateType::Avg;
    } else if (str == "count_distinct") {
        return AggregateType::CountDistinct;
    } else if (str == "min") {
        return AggregateType::Min;
    } else if (str == "max") {
        return AggregateType::Max;
    } else {
        DLOG(ERROR) << "Wrong aggregate type";
        throw std::exception();
    }
}

AggregateOperator::AggregateOperator(
    std::shared_ptr<IOperator> child,
    std::vector<AggregateType> aggregations,
    std::vector<std::shared_ptr<IExpression>> expressions)
    : child_(child), aggregations_(aggregations), expressions_(expressions) {
    if (aggregations_.size() != expressions.size()) {
        DLOG(ERROR) << "Wrong agg/expr size";
        throw std::exception();
    }
}

std::shared_ptr<IAggregateFunc> FuncByAggregateTypeHelper(AggregateType type_,
                                                          Types in_type) {
    switch (type_) {
        case AggregateType::Count: {
            return std::make_shared<CountFunc>(in_type);
        }
        case AggregateType::Sum: {
            return std::make_shared<SumFunc>(in_type);
        }
        case AggregateType::Avg: {
            return std::make_shared<AvgFunc>(in_type);
        }
        case AggregateType::CountDistinct: {
            return std::make_shared<CountDistinctFunc>(in_type);
        }
        case AggregateType::Min: {
            return std::make_shared<MinFunc>(in_type);
        }
        case AggregateType::Max: {
            return std::make_shared<MaxFunc>(in_type);
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
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        Column cl = expressions_[i]->Evaluate(*nw);
        funcs.emplace_back(
            FuncByAggregateTypeHelper(aggregations_[i], cl.GetType()));
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
    std::vector<Types> out_types;
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        res.emplace_back(funcs[i]->Finalize(states[i]));
        out_types.emplace_back(res.back().GetType());
    }
    return Batch(Schema(std::vector<std::string>(res.size(), "asd"), out_types),
                 std::move(res));
}

GroupByOperator::GroupByOperator(
    std::shared_ptr<IOperator> child,
    std::vector<AggregateType> aggregations,
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
    std::vector<std::string> out_names_keys;
    for (auto& i : keys_) {
        out_names_keys.emplace_back(i->GetName());
    }

    std::vector<std::string> out_names_agg(aggregations_.size());
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        Column cl = expressions_[i]->Evaluate(*nw);
        out_names_agg[i] = AggregateTypeToString(aggregations_[i]) + "_" +
                           expressions_[i]->GetName();
        funcs.emplace_back(
            FuncByAggregateTypeHelper(aggregations_[i], cl.GetType()));
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
    std::vector<Types> out_key_types(keys_.size(), Types::kString);
    std::vector<Types> out_types_agg(aggregations_.size());
    for (size_t i = 0; i < aggregations_.size(); ++i) {
        for (auto& [a, b] : states[i]) {
            if (res_aggs[i].GetSize() == 0) {
                res_aggs[i] = funcs[i]->Finalize(b);
                out_types_agg[i] = res_aggs[i].GetType();
            } else {
                res_aggs[i].MergeWithOtherColumn(funcs[i]->Finalize(b));
            }
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
    out_key_types.insert(out_key_types.end(),
                         out_types_agg.begin(),
                         out_types_agg.end());
    res_keys.insert(res_keys.end(), res_aggs.begin(), res_aggs.end());
    out_names_keys.insert(out_names_keys.end(),
                          out_names_agg.begin(),
                          out_names_agg.end());
    // DLOG(INFO) << "group by out batch names";
    // for (auto& name : out_names_keys) {
    //     DLOG(INFO) << name;
    // }
    return Batch(Schema(out_names_keys, out_key_types), std::move(res_keys));
}

class MultiMapBase {
public:
    virtual ~MultiMapBase() = default;
};

template <typename T>
class MultiMap : public MultiMapBase {
public:
    ~MultiMap() override = default;
    std::multimap<T, Batch> map4ik;
};

OrderByOperator::OrderByOperator(std::shared_ptr<IOperator> child,
                                 std::shared_ptr<IExpression> keys,
                                 bool desc)
    : child_(std::move(child)), keys_(std::move(keys)), desc_(desc) {
}

std::optional<Batch> OrderByOperator::Next() {
    if (!child_->Next().has_value()) {
        return std::nullopt;
    }

    std::shared_ptr<MultiMapBase> res_map_ptr;
    std::optional<Batch> nw = child_->Next();
    Column order_cl = keys_->Evaluate(*nw);
    DispatchColumnHelper(order_cl.GetType(), [&res_map_ptr]<Types Src>() {
        using cpptype = typename EnumToCpp<Src>::Type;
        res_map_ptr = std::make_shared<MultiMap<cpptype>>();
    });

    while (nw) {
        order_cl = keys_->Evaluate(*nw);
        DispatchColumnHelper(
            order_cl.GetType(),
            [&res_map_ptr, &nw, &order_cl]<Types Src>() {
                using cpptype = typename EnumToCpp<Src>::Type;
                std::shared_ptr<MultiMap<cpptype>> map_ptr =
                    dynamic_pointer_cast<MultiMap<cpptype>>(res_map_ptr);
                for (size_t i = 0; i < order_cl.GetSize(); ++i) {
                    map_ptr->map4ik.insert(
                        {order_cl.GetElementByIndex<cpptype>(i),
                         nw->GetRow(i)});
                }
            });

        nw = child_->Next();
    }
    Batch res;

    DispatchColumnHelper(
        order_cl.GetType(),
        [&res_map_ptr, &res, this]<Types Src>() {
            using cpptype = typename EnumToCpp<Src>::Type;
            std::shared_ptr<MultiMap<cpptype>> map_ptr =
                dynamic_pointer_cast<MultiMap<cpptype>>(res_map_ptr);
            Batch temp;
            auto fill_temp = [&temp](auto begin, auto end) {
                temp = std::move(begin->second);
                ++begin;
                for (; begin != end; ++begin) {
                    temp.MergeWithOtherBatch(std::move(begin->second));
                }
            };
            if (desc_) {
                fill_temp(map_ptr->map4ik.rbegin(), map_ptr->map4ik.rend());
            } else {
                fill_temp(map_ptr->map4ik.begin(), map_ptr->map4ik.end());
            }
            res = std::move(temp);
        });
    return res;
}

OrderByLimitOperator::OrderByLimitOperator(std::shared_ptr<IOperator> child,
                                           std::shared_ptr<IExpression> keys,
                                           size_t limit,
                                           bool desc)
    : child_(std::move(child)),
      keys_(std::move(keys)),
      limit_(limit),
      desc_(desc) {
}

std::optional<Batch> OrderByLimitOperator::Next() {
    std::optional<Batch> nw = child_->Next();
    if (!nw.has_value()) {
        return std::nullopt;
    }

    std::shared_ptr<MultiMapBase> res_map_ptr;
    Column order_cl = keys_->Evaluate(*nw);
    DispatchColumnHelper(order_cl.GetType(), [&res_map_ptr]<Types Src>() {
        using cpptype = typename EnumToCpp<Src>::Type;
        res_map_ptr = std::make_shared<MultiMap<cpptype>>();
    });

    while (nw) {
        order_cl = keys_->Evaluate(*nw);
        DispatchColumnHelper(
            order_cl.GetType(),
            [&res_map_ptr, &nw, &order_cl, this]<Types Src>() {
                using cpptype = typename EnumToCpp<Src>::Type;
                std::shared_ptr<MultiMap<cpptype>> map_ptr =
                    dynamic_pointer_cast<MultiMap<cpptype>>(res_map_ptr);

                for (size_t i = 0; i < order_cl.GetSize(); ++i) {
                    if (map_ptr->map4ik.size() < limit_) {
                        map_ptr->map4ik.insert(
                            {order_cl.GetElementByIndex<cpptype>(i),
                             nw->GetRow(i)});
                        continue;
                    }
                    cpptype key_nw = order_cl.GetElementByIndex<cpptype>(i);
                    if (desc_) {
                        if (key_nw > map_ptr->map4ik.begin()->first) {
                            map_ptr->map4ik.erase(map_ptr->map4ik.begin());
                            map_ptr->map4ik.insert({key_nw, nw->GetRow(i)});
                        }
                    } else {
                        if (key_nw < map_ptr->map4ik.rbegin()->first) {
                            map_ptr->map4ik.erase(
                                std::prev(map_ptr->map4ik.end()));
                            map_ptr->map4ik.insert({key_nw, nw->GetRow(i)});
                        }
                    }
                }
            });
        nw = child_->Next();
    }
    Batch res;

    DispatchColumnHelper(
        order_cl.GetType(),
        [&res_map_ptr, &res, this]<Types Src>() {
            using cpptype = typename EnumToCpp<Src>::Type;
            std::shared_ptr<MultiMap<cpptype>> map_ptr =
                dynamic_pointer_cast<MultiMap<cpptype>>(res_map_ptr);
            Batch temp;
            auto fill_temp = [&temp](auto begin, auto end) {
                temp = std::move(begin->second);  // may fall if empty
                ++begin;
                for (; begin != end; ++begin) {
                    temp.MergeWithOtherBatch(std::move(begin->second));
                }
            };
            if (desc_) {
                fill_temp(map_ptr->map4ik.rbegin(), map_ptr->map4ik.rend());
            } else {
                fill_temp(map_ptr->map4ik.begin(), map_ptr->map4ik.end());
            }
            res = std::move(temp);
        });
    return res;
}

LimitOperator::LimitOperator(std::shared_ptr<IOperator> child, size_t limit)
    : child_(std::move(child)), limit_(limit) {
}

std::optional<Batch> LimitOperator::Next() {
    size_t count = 0;
    Batch res;
    while (std::optional<Batch> nw = child_->Next()) {
        if (count >= limit_) {
            break;
        }
        for (size_t i = 0; i < nw->GetColumnSize() && count < limit_;
             ++i, ++count) {
            if (count == 0) {
                res = nw->GetRow(i);
            } else {
                res.MergeWithOtherBatch(nw->GetRow(i));
            }
        }
    }
    return res;
}

OffsetOperator::OffsetOperator(std::shared_ptr<IOperator> child, size_t offset)
    : child_(std::move(child)), offset_(offset) {
}

std::optional<Batch> OffsetOperator::Next() {
    size_t count = 0;
    bool has_rows = false;
    Batch res;
    while (std::optional<Batch> nw = child_->Next()) {
        for (size_t i = 0; i < nw->GetColumnSize(); ++i, ++count) {
            if (count < offset_) {
                continue;
            }
            if (!has_rows) {
                res = nw->GetRow(i);
                has_rows = true;
            } else {
                res.MergeWithOtherBatch(nw->GetRow(i));
            }
        }
    }
    if (!has_rows) {
        return std::nullopt;
    }
    return res;
}
