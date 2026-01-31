#include "csv-rw.h"
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <glog/logging.h>
#include <istream>
#include <variant>
#include "batch.h"
#include "column.h"
#include "schema.h"
#include "types.h"

const size_t kMaxStringLenght = 1ull << 31;

CSVReader::CSVReader(std::fstream* input, uint8_t cnt_columns, char delim,
                     bool have_header)
    : input_(input), cnt_columns_(cnt_columns), delim_(delim) {
    if (have_header) {
        GetRow();
    }
}

std::vector<std::string> CSVReader::GetRow() {
    if (IsReaded()) {
        DLOG(ERROR) << "CSV V S E";
        throw std::exception();
    }
    bool is_quote = false;
    int sym = input_->get();
    std::vector<std::string> res;
    res.reserve(cnt_columns_);
    std::string s;
    while (!(sym == EOF ||
             (sym == '\r' && input_->peek() == '\n' && !is_quote))) {
        if (s.size() > kMaxStringLenght) {
            DLOG(ERROR)
                << "i can't work with too big data, one cell is more then "
                << kMaxStringLenght << " bytes";
            throw std::exception();
        }
        if (sym == delim_ && !is_quote) {
            res.emplace_back(std::move(s));
            s.clear();
        } else if (sym == '\"') {
            if (is_quote && input_->peek() == '"') {
                input_->get();
                s.push_back(sym);
            } else {
                is_quote = !is_quote;
            }
        } else {
            s.push_back(sym);
        }
        sym = input_->get();
    }
    input_->get();
    if (input_->peek() == EOF) {
        input_->get();
    }
    res.emplace_back(std::move(s));
    if (is_quote) {
        DLOG(ERROR) << "not closed quote";
        throw std::exception();
    }
    if (res.size() != cnt_columns_) {
        DLOG(ERROR) << "cnt columns neq cnt readed csv";
        throw std::exception();
    }
    return res;
}

Batch CSVReader::GetBatch(size_t batch_row_size) {
    DLOG(INFO) << "CSVReader give batch";
    if (IsReaded()) {
        DLOG(ERROR) << "CSV V S E";
        throw std::exception();
    }

    std::vector<std::vector<std::string>> batch(cnt_columns_);
    for (size_t j = 0; j < cnt_columns_; ++j) {
        batch[j].reserve(batch_row_size);
    }
    for (size_t i = 0; i < batch_row_size && !IsReaded(); ++i) {
        std::vector<std::string> row = GetRow();
        for (size_t j = 0; j < cnt_columns_; ++j) {
            batch[j].emplace_back(std::move(row[j]));
        }
    }
    std::vector<Column> columns(cnt_columns_);
    for (size_t i = 0; i < cnt_columns_; ++i) {
        columns[i] = std::move(batch[i]);
    }
    return Batch(Schema(std::vector<std::string>(cnt_columns_, ""),
                        std::vector<Types>(cnt_columns_, Types::kString)),
                 std::move(columns));
}

bool CSVReader::IsReaded() {
    return input_->eof();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

CSVWriter::CSVWriter(std::fstream* output) : out_(output) {
}

std::string ScreenString(std::string&& s) {
    std::string res = "\"";
    for (size_t i = 0; i < s.size(); ++i) {
        res.push_back(s[i]);
        if (s[i] == '\"') {
            res.push_back(i);
        }
    }
    res.push_back('\"');
    return res;
}

void CSVWriter::WriteBatch(Batch bat) {
    DLOG(INFO) << "write batch";
    bat.NewTypes(std::vector<Types>(bat.GetCntColumns(), Types::kString));
    std::vector<std::vector<std::string>> batch_data(bat.GetCntColumns());

    for (size_t j = 0; j < bat.GetCntColumns(); ++j) {
        Column a = std::move(bat.GetColumnIdx(j));
        std::visit(Overloaded{[&](std::vector<std::string>& v) {
                                  batch_data[j] = std::move(v);
                              },
                              [](auto&& v) {
                                  DLOG(ERROR) << "LOL";
                                  for (auto i : v) {
                                      DLOG(ERROR) << i;
                                  }
                                  throw std::exception();
                              }},
                   a.GetData());
    }

    std::cout << bat.GetColumnSize() << ' ' << bat.GetCntColumns() << '\n';

    for (auto i : batch_data) {
        for (auto j : i) {
            std::cout << j << ' ';
        }
        std::cout << '\n';
    }

    for (size_t i = 0; i < bat.GetColumnSize(); ++i) {
        for (size_t j = 0; j < bat.GetCntColumns(); ++j) {
            std::string screened_str =
                ScreenString(std::move(batch_data[j][i]));
            DLOG_FIRST_N(INFO, 12) << i << ' ' << j << ' ' << screened_str;
            out_->write(screened_str.data(), screened_str.size());
        }
        out_->put('\r');
        out_->put('\n');
    }
    DLOG(INFO) << "wrote batch";
}
