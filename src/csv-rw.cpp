#include "csv-reader.h"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <glog/logging.h>
#include <istream>
#include "batch.h"

const size_t kMaxStringLenght = 1ull << 31;

CSVReader::CSVReader(std::istream *input, uint8_t cnt_columns, char delim,
                     bool have_header)
    : input_(input),
      cnt_columns_(cnt_columns),
      delim_(delim),
      have_header_(have_header) {
}

std::vector<std::string> CSVReader::GetRow() {
    if (IsRead()) {
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

Butch CSVReader::GetButch(size_t batch) {
}

bool CSVReader::IsRead() {
    return input_->eof();
}