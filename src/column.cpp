#include "column.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <type_traits>
#include "rwconsts.h"
#include "types.h"

ColumnType& Column::GetData() & {
    return data_;
}
ColumnType&& Column::GetData() && {
    return std::move(data_);
}

const ColumnType& Column::GetData() const {
    return data_;
}

template <Types Src, Types Dst, typename InputType>
concept ConvertibleTypes = requires(InputType val) {
    { TranslateTtoU<Src, Dst>(val) };
};

void Column::TranslateTo(Types t) {

    DispatchColumnHelper(type_, [&]<Types Src>() {
        DispatchColumnHelper(t, [&]<Types Dst>() {
            std::visit(
                Overloaded{
                    [&](auto&& v) {
                        using VecType = std::decay_t<decltype(v)>;
                        using ValType = typename VecType::value_type;
                        using ExpectedType = typename EnumToCpp<Src>::Type;

                        if constexpr (std::is_same_v<ValType, ExpectedType> &&
                                      ConvertibleTypes<Src, Dst, ValType>) {

                            using ResultType = typename EnumToCpp<Dst>::Type;
                            std::vector<ResultType> res;
                            res.reserve(v.size());

                            for (const auto& item : v) {
                                res.emplace_back(TranslateTtoU<Src, Dst>(item));
                            }
                            data_ = std::move(res);
                        } else {
                            DLOG(ERROR)
                                << "tried translate from " << TypeToString(Src)
                                << " to " << TypeToString(Dst);
                            throw std::exception();
                        }
                    },
                },
                data_);
        });
    });
    type_ = t;
}

size_t Column::GetSize() const {
    size_t res = 0;
    std::visit([&res](auto&& v) { res = v.size(); }, data_);
    return res;
}

Types Column::GetType() const {
    return type_;
}

void Column::PrintCol() {
    DLOG(INFO) << "print col";
    std::visit(Overloaded{[](auto&& v) {
                   for (auto i : v) {
                       DLOG(INFO) << i;
                   }
               }},
               data_);
}

template <typename T>
concept NumericVector = requires(T v) {
    typename T::value_type;
    requires std::is_arithmetic_v<typename T::value_type>;
    requires !std::is_same_v<typename T::value_type, bool>;
    requires std::same_as<T, std::vector<typename T::value_type>>;
};

int64_t WriteColToBzn(std::fstream* file, Column&& c) {

    int64_t res = 0;
    std::visit(Overloaded{
                   [&res, &file]<NumericVector V>(const V& v) {
                       res = v.size() * sizeof(typename V::value_type);
                       file->write(reinterpret_cast<const char*>(v.data()),
                                   res);
                   },
                   [](const std::vector<bool>&) {
                       DLOG(ERROR) << "dont write boolean vector to bzn plz =(";
                       throw std::exception();
                   },
                   [&res, &file](const std::vector<std::string>& v) {
                       for (auto& i : v) {
                           file->write(i.data(), i.size());
                           file->put(kStringDelimiter);
                           res += i.size() + 1;
                       }
                   },
               },
               c.GetData());
    file->flush();
    return res;
}

// Column in half-open [file->tellg, end)
Column ReadColFromBzn(std::fstream* file, Types t, int64_t end) {
    Column res;
    int64_t tellg_nw = file->tellg();  // less syscalls
    DispatchColumnHelper(t, [&]<Types Dst>() {
        using CppType = EnumToCpp<Dst>::Type;
        if constexpr (std::is_same_v<CppType, std::string>) {
            std::vector<std::string> v;
            while (tellg_nw < end) {
                std::string s;
                while (file->peek() != kStringDelimiter) {
                    s.push_back(file->get());
                    tellg_nw++;
                }
                file->get();
                tellg_nw++;
                v.emplace_back(s);
            }
            res = Column(std::move(v), t);
        } else {
            if constexpr (std::is_same_v<CppType, bool>) {
                DLOG(ERROR) << "dont read boolean vector from bzn plz =(";
                throw std::exception();
            } else {
                size_t sz = (end - tellg_nw) / sizeof(CppType);
                std::vector<CppType> v(sz);
                file->read(reinterpret_cast<char*>(v.data()),
                           sz * sizeof(CppType));
                res = Column(std::move(v), t);
            }
        }
    });
    return res;
}
