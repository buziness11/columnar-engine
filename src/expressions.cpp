#include "expressions.h"
#include <cstddef>
#include <exception>
#include <variant>
#include "column.h"
#include "types.h"

ColumnRef::ColumnRef(const std::string& name) : column_name_(name) {
}

Column ColumnRef::Evaluate(const Batch& batch) {
    return batch.GetColumnByName(column_name_);
}

BinaryCmp::BinaryCmp(std::shared_ptr<IExpression> left, CmpType cmp_type,
                     std::shared_ptr<IExpression> right)
    : left_(left), cmp_type_(cmp_type), right_(right) {
}

Column BinaryCmp::Evaluate(const Batch& b) {
    Column l = left_->Evaluate(b);
    Column r = right_->Evaluate(b);
    Column res;
    std::visit(
        Overloaded{[&res, &r, this](auto&& lv) {
            std::visit(
                Overloaded{[&res, &lv, this](auto&& rv) {
                    using LType = std::decay_t<decltype(lv)>::value_type;
                    using RType = std::decay_t<decltype(rv)>::value_type;
                    if constexpr (std::equality_comparable_with<LType, RType>) {
                        std::vector<bool> a(rv.size());
                        for (size_t i = 0; i < rv.size(); ++i) {
                            switch (this->cmp_type_) {
                                case CmpType::Neq: {
                                    a[i] = (lv[i] != rv[i]);
                                    break;
                                }
                                default: {
                                    DLOG(ERROR) << "dont support this cmp";
                                    throw std::exception();
                                }
                            }
                        }
                        res = Column(a, Types::kBool);
                    } else {
                        DLOG(ERROR) << "Cannot compare such types";
                        throw std::exception();
                    }
                }},
                r.GetData());
        }},
        l.GetData());
    return res;
}