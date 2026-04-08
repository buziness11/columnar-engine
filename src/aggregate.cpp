#include "aggregate.h"
#include "column.h"
#include "types.h"
#include <cstdint>
#include <exception>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

std::shared_ptr<IAggregateState> CountFunc::CreateState() {
    return std::make_shared<CountState>();
}

void CountFunc::Update(std::shared_ptr<IAggregateState> state,
                       const Column& col) {
    std::shared_ptr<CountState> ptr =
        std::dynamic_pointer_cast<CountState>(state);
    if (ptr) {
        ptr->res += col.GetSize();
    } else {
        throw std::exception();
    }
}

Column CountFunc::Finalize(std::shared_ptr<IAggregateState> state, Types) {
    std::shared_ptr<CountState> ptr =
        std::dynamic_pointer_cast<CountState>(state);
    return Column(std::vector<int32_t>(1, ptr->res), Types::kInt32_t);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

SumFunc::SumFunc(Types type) : type_(type) {
}

std::shared_ptr<IAggregateState> SumFunc::CreateState() {
    if (type_ == Types::kInt16_t || type_ == Types::kInt32_t ||
        type_ == Types::kInt64_t) {
        return std::make_shared<SumState<int64_t>>();
    } else if (type_ == Types::kDouble || type_ == Types::kLongDouble) {
        return std::make_shared<SumState<long double>>();
    }
    DLOG(ERROR) << "Cannot create sum state for type: " << TypeToString(type_);
    throw std::exception();
}

template <typename T>
concept IntVector = requires(T v) {
    typename T::value_type;
    requires std::is_arithmetic_v<typename T::value_type>;
    requires std::is_same_v<typename T::value_type, int64_t> ||
                 std::is_same_v<typename T::value_type, int32_t> ||
                 std::is_same_v<typename T::value_type, int16_t>;
    requires std::same_as<T, std::vector<typename T::value_type>>;
};

template <typename T>
concept DoubleVector = requires(T v) {
    typename T::value_type;
    requires std::is_arithmetic_v<typename T::value_type>;
    requires std::is_same_v<typename T::value_type, double> ||
                 std::is_same_v<typename T::value_type, long double>;
    requires std::same_as<T, std::vector<typename T::value_type>>;
};

void SumFunc::Update(std::shared_ptr<IAggregateState> state,
                     const Column& col) {
    std::visit(
        Overloaded{[&state]<IntVector V>(const V& v) {
                       std::shared_ptr<SumState<int64_t>> ptr =
                           std::dynamic_pointer_cast<SumState<int64_t>>(state);
                       for (auto i : v) {
                           ptr->res += i;
                       }
                   },
                   [&state]<DoubleVector V>(const V& v) {
                       std::shared_ptr<SumState<long double>> ptr =
                           std::dynamic_pointer_cast<SumState<long double>>(
                               state);
                       for (auto i : v) {
                           ptr->res += i;
                       }
                   },
                   [&col](auto&&) {
                       DLOG(ERROR) << "Dont sum aggregate column with type: "
                                   << TypeToString(col.GetType());
                   }},
        col.GetData());
}

Column SumFunc::Finalize(std::shared_ptr<IAggregateState> state, Types) {
    if (type_ == Types::kInt16_t || type_ == Types::kInt32_t ||
        type_ == Types::kInt64_t) {
        std::shared_ptr<SumState<int64_t>> ptr =
            std::dynamic_pointer_cast<SumState<int64_t>>(state);
        return Column(std::vector<int64_t>{ptr->res}, Types::kInt64_t);
    } else if (type_ == Types::kDouble || type_ == Types::kLongDouble) {
        std::shared_ptr<SumState<long double>> ptr =
            std::dynamic_pointer_cast<SumState<long double>>(state);
        return Column(std::vector<long double>{ptr->res}, Types::kLongDouble);
    }
    DLOG(ERROR) << "Cannot finalize sum state for type: "
                << TypeToString(type_);
    throw std::exception();
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<IAggregateState> AvgFunc::CreateState() {
    return std::make_shared<AvgState>();
}

template <typename T>
concept NumericVector = requires(T v) {
    typename T::value_type;
    requires std::is_arithmetic_v<typename T::value_type>;
    requires !std::is_same_v<typename T::value_type, bool>;
    requires std::same_as<T, std::vector<typename T::value_type>>;
};

void AvgFunc::Update(std::shared_ptr<IAggregateState> state,
                     const Column& col) {
    std::shared_ptr<AvgState> ptr = std::dynamic_pointer_cast<AvgState>(state);
    if (!ptr) {
        DLOG(ERROR) << "Wrong avg state in update";
        throw std::exception();
    }
    ptr->cnt += col.GetSize();

    std::visit(Overloaded{[&ptr]<NumericVector V>(const V& v) {
                              long double accumulate = 0;
                              for (auto i : v) {
                                  accumulate += i;
                              }
                              ptr->sum += accumulate;
                          },
                          [&col](auto&&) {
                              DLOG(ERROR)
                                  << "Dont avg aggregate column with type: "
                                  << TypeToString(col.GetType());
                          }},
               col.GetData());
}

Column AvgFunc::Finalize(std::shared_ptr<IAggregateState> state, Types) {
    std::shared_ptr<AvgState> ptr = std::dynamic_pointer_cast<AvgState>(state);
    if (!ptr) {
        DLOG(ERROR) << "Wrong avg state in finalize";
        throw std::exception();
    }
    return Column(
        std::vector<long double>{ptr->sum / static_cast<long double>(ptr->cnt)},
        Types::kLongDouble);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

CountDistinctFunc::CountDistinctFunc(Types type) : type_(type) {
}

std::shared_ptr<IAggregateState> CountDistinctFunc::CreateState() {
    std::shared_ptr<IAggregateState> res;
    DispatchColumnHelper(type_, [&res]<Types Dst>() {
        using cpptype = EnumToCpp<Dst>::Type;
        res = std::make_shared<CountDistinctState<cpptype>>();
    });
    return res;
}

void CountDistinctFunc::Update(std::shared_ptr<IAggregateState> state,
                               const Column& col) {

    std::visit(
        Overloaded{[this, &state](auto&& v) {
            DispatchColumnHelper(type_, [&state, &v]<Types Dst>() {
                using cpptype = EnumToCpp<Dst>::Type;
                if constexpr (std::is_same_v<typename std::decay_t<
                                                 decltype(v)>::value_type,
                                             cpptype>) {
                    std::shared_ptr<CountDistinctState<cpptype>> ptr =
                        std::dynamic_pointer_cast<CountDistinctState<cpptype>>(
                            state);
                    for (cpptype i : v) {
                        (ptr->res).insert(i);
                    }
                }
            });
        }},
        col.GetData());
}

Column CountDistinctFunc::Finalize(std::shared_ptr<IAggregateState> state,
                                   Types) {
    Column res;
    DispatchColumnHelper(type_, [&state, &res]<Types Dst>() {
        using cpptype = EnumToCpp<Dst>::Type;
        std::shared_ptr<CountDistinctState<cpptype>> ptr =
            std::dynamic_pointer_cast<CountDistinctState<cpptype>>(state);
        res =
            Column(std::vector<int32_t>{static_cast<int32_t>(ptr->res.size())},
                   Types::kInt32_t);
    });
    return res;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

MinFunc::MinFunc(Types type) : type_(type) {
}

std::shared_ptr<IAggregateState> MinFunc::CreateState() {
    std::shared_ptr<IAggregateState> res;
    DispatchColumnHelper(type_, [&res]<Types Dst>() {
        if constexpr (Dst == Types::kString || Dst == Types::kBool) {
            DLOG(ERROR) << "Get min from string or type";
            throw std::exception();
        } else {
            using cpptype = EnumToCpp<Dst>::Type;
            res = std::make_shared<MinState<cpptype>>(std::numeric_limits<cpptype>::max());
        }
    });
    return res;
}

void MinFunc::Update(std::shared_ptr<IAggregateState> state,
                     const Column& col) {
    std::visit(
        Overloaded{[this, &state](auto&& v) {
            DispatchColumnHelper(type_, [&state, &v]<Types Dst>() {
                using cpptype = EnumToCpp<Dst>::Type;
                if constexpr (std::is_same_v<typename std::decay_t<
                                                 decltype(v)>::value_type,
                                             cpptype>) {
                    std::shared_ptr<MinState<cpptype>> ptr =
                        std::dynamic_pointer_cast<MinState<cpptype>>(state);
                    for (cpptype i : v) {
                        ptr->res = std::min(ptr->res, i);
                    }
                }
            });
        }},
        col.GetData());
}

Column MinFunc::Finalize(std::shared_ptr<IAggregateState> state, Types) {
    Column res;
    DispatchColumnHelper(type_, [&state, &res, this]<Types Dst>() {
        using cpptype = EnumToCpp<Dst>::Type;
        std::shared_ptr<MinState<cpptype>> ptr =
            std::dynamic_pointer_cast<MinState<cpptype>>(state);
        res = Column(std::vector<cpptype>{ptr->res}, type_);
    });
    return res;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////


MaxFunc::MaxFunc(Types type) : type_(type) {
}

std::shared_ptr<IAggregateState> MaxFunc::CreateState() {
    std::shared_ptr<IAggregateState> res;
    DispatchColumnHelper(type_, [&res]<Types Dst>() {
        if constexpr (Dst == Types::kString || Dst == Types::kBool) {
            DLOG(ERROR) << "Get max from string or type";
            throw std::exception();
        } else {
            using cpptype = EnumToCpp<Dst>::Type;
            res = std::make_shared<MaxState<cpptype>>(std::numeric_limits<cpptype>::min());
        }
    });
    return res;
}

void MaxFunc::Update(std::shared_ptr<IAggregateState> state,
                     const Column& col) {
    std::visit(
        Overloaded{[this, &state](auto&& v) {
            DispatchColumnHelper(type_, [&state, &v]<Types Dst>() {
                using cpptype = EnumToCpp<Dst>::Type;
                if constexpr (std::is_same_v<typename std::decay_t<
                                                 decltype(v)>::value_type,
                                             cpptype>) {
                    std::shared_ptr<MaxState<cpptype>> ptr =
                        std::dynamic_pointer_cast<MaxState<cpptype>>(state);
                    for (cpptype i : v) {
                        ptr->res = std::max(ptr->res, i);
                    }
                }
            });
        }},
        col.GetData());
}

Column MaxFunc::Finalize(std::shared_ptr<IAggregateState> state, Types) {
    Column res;
    DispatchColumnHelper(type_, [&state, &res, this]<Types Dst>() {
        using cpptype = EnumToCpp<Dst>::Type;
        std::shared_ptr<MaxState<cpptype>> ptr =
            std::dynamic_pointer_cast<MaxState<cpptype>>(state);
        res = Column(std::vector<cpptype>{ptr->res}, type_);
    });
    return res;
}
