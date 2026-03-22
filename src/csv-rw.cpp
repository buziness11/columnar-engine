#include "csv-rw.h"
#include <functional>
#include <iostream>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <glog/logging.h>
#include <istream>
#include <variant>
#include "batch.h"
#include "column.h"
#include "schema.h"
#include "types.h"
#include "rwconsts.h"

CSVReader::CSVReader(std::fstream* input, size_t cnt_columns, char delim,
                     bool lf, bool have_header)
    : input_(input), cnt_columns_(cnt_columns), delim_(delim), lf_(lf) {
    if (have_header) {
        GetRow();
    }
}

inline bool PredicateCRLF(int sym, int peek, bool is_quote) {
    return !(sym == EOF || (sym == '\r' && peek == '\n' && !is_quote));
}

inline bool PredicateLF(int sym, int, bool is_quote) {
    return !(sym == EOF || (sym == '\n' && !is_quote));
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
    std::function<bool(int, int, bool)> predicate = PredicateLF;
    if (!lf_) {
        predicate = PredicateCRLF;
    }
    while (predicate(sym, input_->peek(), is_quote)) {
        if (res.size() > cnt_columns_) {
            DLOG(ERROR) << "Stop reading, mb wrong csv format\ncnt columns neq "
                           "cnt readed csv";
            throw std::exception();
        }
        if (s.size() > kMaxStringLenghtCsvSize) {
            DLOG(ERROR)
                << "i can't work with too big data, one cell is more then "
                << kMaxStringLenghtCsvSize << " bytes";
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
    if (!lf_) {
        input_->get();
    }
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
        columns[i] = Column(batch[i], Types::kString);
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

CSVWriter::CSVWriter(std::fstream* output, bool lf) : out_(output), lf_(lf) {
}

std::string ScreenString(std::string&& s) {
    std::string res = "\"";
    res += s;
    res.push_back('\"');
    return res;
}

void CSVWriter::WriteBatch(Batch bat, char delim) {
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
    if (batch_data.empty()) {
        DLOG(WARNING) << "write empty batch in csv";
        return;
    }

    for (size_t i = 0; i < batch_data[0].size(); ++i) {
        std::string screened_str;
        for (int j = 0; j < static_cast<int>(batch_data.size()) - 1; ++j) {
            screened_str = ScreenString(std::move(batch_data[j][i]));
            out_->write(screened_str.data(), screened_str.size());
            out_->put(delim);
        }
        screened_str = ScreenString(std::move(batch_data.back()[i]));
        out_->write(screened_str.data(), screened_str.size());
        if (!lf_) {
            out_->put('\r');
        }
        out_->put('\n');
    }
    out_->flush();
}
