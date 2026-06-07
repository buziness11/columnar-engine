#include "query/expressions.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <boost/regex.hpp>
#include "core/column.h"
#include "core/datatype.h"
#include "core/types.h"

ColumnRef::ColumnRef(const std::string& name) : name_(name) {
}

Column ColumnRef::Evaluate(const Batch& batch) {
    return batch.GetColumnByName(name_);
}

std::string ColumnRef::GetName() const {
    return name_;
}

BinaryCmp::BinaryCmp(std::shared_ptr<IExpression> left,
                     CmpType cmp_type,
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

BinaryFunc::BinaryFunc(std::shared_ptr<IExpression> left,
                       FuncType bin_func_type,
                       std::shared_ptr<IExpression> right,
                       const std::string& name)
    : left_(left), bin_func_type_(bin_func_type), right_(right), name_(name) {
}

Column BinaryFunc::Evaluate(const Batch& b) {
    Column res;
    Column l = left_->Evaluate(b);
    Column r = right_->Evaluate(b);
    DispatchColumnHelper(l.GetType(), [&l, &r, &res, this]<Types L>() {
        if constexpr (L != Types::kString && L != Types::kTimestamp &&
                      L != Types::kDate && L != Types::kLongDouble &&
                      L != Types::kDouble) {
            DispatchColumnHelper(r.GetType(), [&l, &r, &res, this]<Types R>() {
                if constexpr (R != Types::kString && R != Types::kTimestamp &&
                              R != Types::kDate && R != Types::kLongDouble &&
                              R != Types::kDouble) {
                    using type_l = typename EnumToCpp<L>::Type;
                    using type_r = typename EnumToCpp<R>::Type;
                    using type_out =
                        std::conditional_t<std::is_same_v<type_l, type_r> &&
                                               std::is_same_v<type_l, bool>,
                                           bool,
                                           decltype(std::declval<type_l>() +
                                                    std::declval<type_r>())>;
                    std::function<type_out(type_l, type_r)> ma_func;
                    switch (bin_func_type_) {
                        case FuncType::Plus:
                            ma_func = [](type_l lhs, type_r rhs) {
                                return static_cast<type_out>(lhs + rhs);
                            };
                            break;
                        case FuncType::Minus:
                            ma_func = [](type_l lhs, type_r rhs) {
                                return static_cast<type_out>(lhs - rhs);
                            };
                            break;
                        case FuncType::Or:
                            ma_func = [](type_l lhs, type_r rhs) {
                                return static_cast<type_out>(lhs | rhs);
                            };
                            break;
                        case FuncType::And:
                            ma_func = [](type_l lhs, type_r rhs) {
                                return static_cast<type_out>(lhs & rhs);
                            };
                            break;
                    }

                    std::vector<type_out> a;
                    a.reserve(l.GetSize());
                    for (size_t i = 0; i < l.GetSize(); i++) {
                        a.emplace_back(ma_func(l.GetElementByIndex<type_l>(i),
                                               r.GetElementByIndex<type_r>(i)));
                    }
                    res = Column(std::move(a), CppToEnum<type_out>::value);
                }
            });
        }
    });
    return res;
}

std::string BinaryFunc::GetName() const {
    return name_;
}

Like::Like(std::shared_ptr<IExpression> child,
           const std::string& pattern,
           bool invert,
           const std::string& name)
    : child_(child), pattern_(pattern), invert_(invert), name_(name) {
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
    std::vector<bool> res(strs.size(), false ^ invert_);
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
                res[i] = true ^ invert_;
                break;
            }
        }
    }
    return Column(std::move(res), Types::kBool);
}

std::string Like::GetName() const {
    return name_;
}

ExtractFromTime::ExtractFromTime(std::shared_ptr<IExpression> child,
                                 //  TimeExtractType time_extract_type,
                                 const std::string& name)
    : child_(child), name_(name) {
}

Column ExtractFromTime::Evaluate(const Batch& b) {
    Column c = child_->Evaluate(b);
    std::vector<int64_t> ts =
        std::move(std::get<std::vector<int64_t>>(std::move(c.GetData())));
    std::vector<int32_t> res(ts.size());
    for (size_t i = 0; i < ts.size(); i++) {
        res[i] = extract_func_(ts[i]);
    }
    return Column(std::move(res), Types::kInt32_t);
}

std::string ExtractFromTime::GetName() const {
    return name_;
}

TruncateTime::TruncateTime(std::shared_ptr<IExpression> child,
                           Trunc trunc,
                           const std::string& name)
    : child_(child), trunc_(trunc), name_(name) {
}

Column TruncateTime::Evaluate(const Batch& b) {
    Column c = child_->Evaluate(b);
    if (c.GetType() != Types::kTimestamp) {
        DLOG(ERROR) << "Cannot truncate non-timestamp type";
    }
    std::vector<int64_t> ts =
        std::move(std::get<std::vector<int64_t>>(std::move(c.GetData())));
    std::vector<int64_t> res(ts.size());
    for (size_t i = 0; i < ts.size(); i++) {
        res[i] = TruncateTimestamp(ts[i], trunc_);
    }
    return Column(std::move(res), Types::kTimestamp);
}

std::string TruncateTime::GetName() const {
    return name_;
}

Length::Length(std::shared_ptr<IExpression> child, const std::string& name)
    : child_(child), name_(name) {
}

Column Length::Evaluate(const Batch& b) {
    Column c = child_->Evaluate(b);
    if (c.GetType() != Types::kString) {
        DLOG(ERROR) << "Cannot get length of non-string type";
        throw std::runtime_error("Length: non-string type");
    }
    std::vector<std::string> strs =
        std::move(std::get<std::vector<std::string>>(std::move(c.GetData())));
    std::vector<int32_t> res(strs.size());
    for (size_t i = 0; i < strs.size(); ++i) {
        res[i] = static_cast<int32_t>(strs[i].size());
    }
    return Column(std::move(res), Types::kInt32_t);
}

std::string Length::GetName() const {
    return name_;
}

Case::Case(std::shared_ptr<IExpression> child_predicate,
           std::shared_ptr<IExpression> child_true,
           std::shared_ptr<IExpression> child_false,
           const std::string& name)
    : child_predicate_(child_predicate),
      child_true_(child_true),
      child_false_(child_false),
      name_(name) {
}

Column Case::Evaluate(const Batch& b) {
    Column predicate = child_predicate_->Evaluate(b);
    Column when_true = child_true_->Evaluate(b);
    Column when_false = child_false_->Evaluate(b);

    if (predicate.GetType() != Types::kBool) {
        DLOG(ERROR) << "Case predicate must be bool";
        throw std::runtime_error("Case: predicate must be bool");
    }
    if (when_true.GetType() != when_false.GetType()) {
        DLOG(ERROR) << "Case branches must have same type";
        throw std::runtime_error("Case: branch types differ");
    }
    if (predicate.GetSize() != when_true.GetSize() ||
        predicate.GetSize() != when_false.GetSize()) {
        DLOG(ERROR) << "Case branch sizes differ";
        throw std::runtime_error("Case: branch sizes differ");
    }

    std::vector<bool> pred =
        std::move(std::get<std::vector<bool>>(std::move(predicate.GetData())));
    Column res;
    DispatchColumnHelper(
        when_true.GetType(),
        [&when_true, &when_false, &pred, &res]<Types T>() {
            using CppType = typename EnumToCpp<T>::Type;
            std::vector<CppType> true_values = std::move(
                std::get<std::vector<CppType>>(std::move(when_true.GetData())));
            std::vector<CppType> false_values =
                std::move(std::get<std::vector<CppType>>(
                    std::move(when_false.GetData())));
            std::vector<CppType> out;
            out.reserve(pred.size());
            for (size_t i = 0; i < pred.size(); ++i) {
                out.emplace_back(pred[i] ? true_values[i] : false_values[i]);
            }
            res = Column(std::move(out), T);
        });
    return res;
}

std::string Case::GetName() const {
    return name_;
}

RegexpReplace::RegexpReplace(std::shared_ptr<IExpression> child,
                             const std::string& pattern,
                             const std::string& replacement,
                             const std::string& name)
    : child_(child), pattern_(pattern), replacement_(replacement), name_(name) {
}

Column RegexpReplace::Evaluate(const Batch& b) {
    Column c = child_->Evaluate(b);
    if (c.GetType() != Types::kString) {
        DLOG(ERROR) << "RegexpReplace works only with strings";
        throw std::runtime_error("RegexpReplace: non-string type");
    }

    boost::regex expr(pattern_);
    std::vector<std::string> strs =
        std::move(std::get<std::vector<std::string>>(std::move(c.GetData())));
    std::vector<std::string> res;
    res.reserve(strs.size());
    for (const auto& s : strs) {
        res.emplace_back(boost::regex_replace(s, expr, replacement_));
    }
    return Column(std::move(res), Types::kString);
}

std::string RegexpReplace::GetName() const {
    return name_;
}
