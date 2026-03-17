#include "schema.h"
#include <cstddef>
#include <exception>
#include "csv-rw.h"
#include "types.h"

Schema::Schema() : cnt_columns_{0}, column_names_{}, column_types_{} {
}

Schema::Schema(std::fstream* schema, char schema_delim, bool lf)
    : cnt_columns_(0) {
    CSVReader cr(schema, 2, schema_delim, lf);
    while (!cr.IsReaded()) {
        std::vector<std::string> row = cr.GetRow();
        column_names_.emplace_back(std::move(row[0]));
        column_types_.emplace_back(StringToType(row[1]));
        cnt_columns_++;
    }
}

Schema::Schema(std::vector<std::string> names, std::vector<Types> types)
    : column_names_(names), column_types_(types) {
    if (names.size() != types.size()) {
        std::string lg_names = ".";
        for (auto i : names) {
            lg_names += i + ".";
        }
        std::string lg_types = ".";
        for (auto i : types) {
            lg_types += TypeToString(i) + ",";
        }
        DLOG(ERROR) << "Wrong names types sizes for schema\n"
                    << lg_names << '\n'
                    << lg_types << '\n';
        throw std::exception();
    }
    cnt_columns_ = column_types_.size();
}

const std::vector<std::string>& Schema::GetNames() const {
    return column_names_;
}

const std::vector<Types>& Schema::GetTypes() const {
    return column_types_;
}

size_t Schema::GetCntColumns() const {
    return cnt_columns_;
}

Types Schema::GetType(size_t idx) const {
    if (idx >= column_types_.size()) {
        DLOG(ERROR) << "too big index for such schema";
        throw ::std::exception();
    }
    return column_types_[idx];
}

void Schema::SetTypes(const std::vector<Types>& new_tps) {
    column_types_ = new_tps;
}
