#include "column.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include "rwconsts.h"
#include "types.h"

ColumnType& Column::GetData() & {
    return data_;
}
ColumnType&& Column::GetData() && {
    return std::move(data_);
}

Column::Column(Column&& ot) : data_(std::move(ot.data_)), type_(ot.type_) {
}

Column& Column::operator=(Column&& ot) {
    data_ = std::move(ot.data_);
    type_ = ot.type_;
    return *this;
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

size_t Column::GetSize() {
    size_t res = 0;
    std::visit([&res](auto&& v) { res = v.size(); }, data_);
    return res;
}

Types Column::GetType() {
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

int64_t WriteColToBzn(std::fstream* file, Column&& c) {
    int64_t res = 0;
    std::visit(
        Overloaded{
            [&res, &file](const std::vector<int16_t>& v) {
                res = v.size() * sizeof(int16_t);
                file->write(reinterpret_cast<const char*>(v.data()), res);
            },
            [&res, &file](const std::vector<int32_t>& v) {
                res = v.size() * sizeof(int32_t);
                file->write(reinterpret_cast<const char*>(v.data()), res);
            },
            [&res, &file](const std::vector<int64_t>& v) {
                res = v.size() * sizeof(int64_t);
                file->write(reinterpret_cast<const char*>(v.data()), res);
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
            size_t sz = (end - tellg_nw) / sizeof(CppType);
            std::vector<CppType> v(sz);
            file->read(reinterpret_cast<char*>(v.data()), sz * sizeof(CppType));
            res = Column(std::move(v), t);
        }
    });
    return res;
}
