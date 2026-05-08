#include "query/expressions.h"
#include <cstddef>
#include <exception>
#include <string>
#include <variant>
#include "core/column.h"
#include "core/types.h"

ColumnRef::ColumnRef(const std::string& name) : name_(name) {
}

Column ColumnRef::Evaluate(const Batch& batch) {
    return batch.GetColumnByName(name_);
}

std::string ColumnRef::GetName() const {
    return name_;
}

BinaryCmp::BinaryCmp(std::shared_ptr<IExpression> left, CmpType cmp_type,
                     std::shared_ptr<IExpression> right,
                     const std::string& name)
    : left_(left), cmp_type_(cmp_type), right_(right), name_(name) {
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
                    if (lv.size() != rv.size()) {
                        DLOG(ERROR)
                            << "BinaryCmp: column sizes differ: " << lv.size()
                            << " vs " << rv.size();
                        throw std::runtime_error(
                            "BinaryCmp: column sizes differ");
                    }
                    if constexpr (std::equality_comparable_with<LType, RType>) {
                        constexpr bool ordered =
                            std::totally_ordered_with<LType, RType>;
                        std::vector<bool> a(rv.size());
                        for (size_t i = 0; i < rv.size(); ++i) {
                            switch (this->cmp_type_) {
                                case CmpType::Eq:
                                    a[i] = (lv[i] == rv[i]);
                                    break;
                                case CmpType::Neq:
                                    a[i] = (lv[i] != rv[i]);
                                    break;
                                case CmpType::L:
                                    if constexpr (ordered) {
                                        a[i] = (lv[i] < rv[i]);
                                    } else {
                                        DLOG(ERROR) << "types are not "
                                                       "totally_ordered";
                                        throw std::runtime_error(
                                            "BinaryCmp: types are not "
                                            "totally_ordered");
                                    }
                                    break;
                                case CmpType::Leq:
                                    if constexpr (ordered) {
                                        a[i] = (lv[i] <= rv[i]);
                                    } else {
                                        DLOG(ERROR) << "types are not "
                                                       "totally_ordered";
                                        throw std::runtime_error(
                                            "BinaryCmp: types are not "
                                            "totally_ordered");
                                    }
                                    break;
                                case CmpType::G:
                                    if constexpr (ordered) {
                                        a[i] = (lv[i] > rv[i]);
                                    } else {
                                        DLOG(ERROR) << "types are not "
                                                       "totally_ordered";
                                        throw std::runtime_error(
                                            "BinaryCmp: types are not "
                                            "totally_ordered");
                                    }
                                    break;
                                case CmpType::Geq:
                                    if constexpr (ordered) {
                                        a[i] = (lv[i] >= rv[i]);
                                    } else {
                                        DLOG(ERROR) << "types are not "
                                                       "totally_ordered";
                                        throw std::runtime_error(
                                            "BinaryCmp: types are not "
                                            "totally_ordered");
                                    }
                                    break;
                            }
                        }
                        res = Column(std::move(a), Types::kBool);
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

std::string BinaryCmp::GetName() const {
    return name_;
}

Like::Like(std::shared_ptr<IExpression> child, const std::string& pattern,
           const std::string& name)
    : child_(child), pattern_(pattern), name_(name) {
}

std::vector<int> prefix_function(const std::string& t) {
    int n = t.size();
    std::vector<int> p(n, 0);
    for (int i = 1; i < n; i++) {
        int k = p[i - 1];
        while (k > 0 && t[i] != t[k]) {
            k = p[k - 1];
        }
        if (t[i] == t[k]) {
            k++;
        }
        p[i] = k;
    }
    return p;
}

Column Like::Evaluate(const Batch& b) {
    Column c = child_->Evaluate(b);
    std::vector<std::string> strs =
        std::move(std::get<std::vector<std::string>>(std::move(c.GetData())));
    std::vector<bool> res(strs.size());
    std::vector<int> pref = prefix_function(pattern_);
    for (size_t i = 0; i < strs.size(); i++) {
        size_t k = 0;
        for (size_t j = 0; j < strs[i].size(); j++) {
            while (k > 0 && strs[i][j] != pattern_[k]) {
                k = pref[k - 1];
            }
            if (strs[i][j] == pattern_[k])
                k++;
            if (k == pattern_.size()) {
                res[i] = true;
                break;
            }
        }
    }
    return Column(std::move(res), Types::kBool);
}

std::string Like::GetName() const {
    return name_;
}
