#include "column.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include "types.h"

ColumnType& Column::GetData() {
    return data_;
}

void Column::TranslateTo(Types t) {
    std::visit(Overloaded{
                   [&](std::vector<int64_t>& v) {
                       switch (t) {
                           case Types::kInt64_t: {
                               break;
                           }
                           case Types::kString: {
                               std::vector<std::string> newvec(v.size());
                               for (size_t i = 0; i < v.size(); ++i) {
                                   newvec[i] = std::to_string(v[i]);
                               }
                               data_ = std::move(newvec);
                               break;
                           }
                       }
                   },
                   [&](std::vector<std::string>& v) {
                       switch (t) {
                           case Types::kInt64_t: {
                               std::vector<int64_t> newvec(v.size());
                               for (size_t i = 0; i < v.size(); ++i) {
                                   DLOG_FIRST_N(INFO, 2) << v[i];
                                   newvec[i] = stoll(v[i]);
                               }
                               data_ = std::move(newvec);
                               break;
                           }
                           case Types::kString: {
                               break;
                           }
                       }
                   },
               },
               data_);
}

size_t Column::GetSize() {
    size_t res = 0;
    std::visit([&res](auto&& v) { res = v.size(); }, data_);
    return res;
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
