#include "my-format.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <vector>
#include "batch.h"
#include "column.h"
#include "csv-rw.h"
#include "schema.h"
#include "types.h"
#include <glog/logging.h>

char buf_read_int64[8];
int64_t ReadInt64(std::fstream* f) {
    int64_t res;
    f->read(buf_read_int64, sizeof(res));
    memcpy(&res, buf_read_int64, sizeof(res));
    return res;
}

BZNReader::BZNReader(std::fstream* file) : ma_format_(file) {
    DLOG(INFO) << "Getting metasize ...";
    ma_format_->seekg(0, std::ios::beg);
    int64_t metasize = ReadInt64(ma_format_);
    ma_format_->seekg(-metasize, std::ios::end);

    DLOG(INFO) << "Getting schema ...";
    BuildSchema();
    DLOG(INFO) << "Getting offsets ...";
    GetMetaOffset();
    ma_format_->seekg(-metasize, std::ios::end);
    offsets_.emplace_back(ma_format_->tellg());
    DLOG_FIRST_N(INFO, 1) << "Offsets";
    for (auto i : offsets_) {
        DLOG_FIRST_N(INFO, 10) << i;
    }

    ma_format_->seekg(8, std::ios::beg);
    cur_batch_ = 0;
}

void BZNReader::BuildSchema() {
    std::vector<std::string> names = GetMetaString();
    std::vector<std::string> typess = GetMetaString();
    std::vector<Types> types;
    types.reserve(types.size());
    for (auto t : typess) {
        types.emplace_back(StringToType(t));
    }
    schema_ = Schema(names, types);
}

void BZNReader::GetMetaOffset() {
    while (ma_format_->peek() != EOF) {
        offsets_.emplace_back(ReadInt64(ma_format_));
    }
}

std::vector<std::string> BZNReader::GetMetaString() {
    std::vector<std::string> res;
    while (ma_format_->peek() != kMetaDelimiter) {
        std::string s;
        while (ma_format_->peek() != kStringDelimiter) {
            s.push_back(ma_format_->get());
        }
        ma_format_->get();
        res.emplace_back(s);
    }
    ma_format_->get();
    return res;
}

std::vector<int64_t> BZNReader::GetMetaBatchOffset() {
    std::vector<int64_t> dataoffset(schema_.GetCntColumns());
    for (uint8_t i = 0; i < schema_.GetCntColumns(); ++i) {
        dataoffset[i] = ReadInt64(ma_format_);
    }
    return dataoffset;
}

bool BZNReader::IsReaded() {
    return ma_format_->tellg() >= offsets_.back();
}

Column ReadColumn(std::fstream* file, Types t, int64_t end) {
    Column res;
    switch (t) {
        case Types::kInt64_t: {
            std::vector<int64_t> v;
            while (file->tellg() < end) {
                v.emplace_back(ReadInt64(file));
            }
            res = Column(v);
            break;
        }
        case Types::kString: {
            std::vector<std::string> v;
            while (file->tellg() < end) {
                std::string s;
                while (file->peek() != kStringDelimiter) {
                    s.push_back(file->get());
                }
                file->get();
                v.emplace_back(s);
            }
            res = Column(v);
            break;
        }
        default: {
            DLOG(ERROR) << "FORGOT Add Cases";
            throw std::exception();
        }
    }
    return res;
}

Batch BZNReader::Read() {
    if (IsReaded()) {
        DLOG(ERROR) << "all batches readed";
        throw std::exception();
    }
    ma_format_->seekg(offsets_[cur_batch_], std::ios::beg);
    std::vector<int64_t> batch_offset = GetMetaBatchOffset();
    std::vector<Column> bat;

    for (size_t i = 0; i < schema_.GetCntColumns() - 1; ++i) {
        bat.emplace_back(ReadColumn(ma_format_, schema_.GetType(i),
                                    batch_offset[i + 1]));
        bat.back().PrintCol();
    }
    bat.emplace_back(ReadColumn(
        ma_format_, schema_.GetType(schema_.GetCntColumns() - 1),
        offsets_[++cur_batch_]));
    bat.back().PrintCol();
    return Batch(schema_, std::move(bat));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char buf_write_int64[8];
void WriteInt64(std::fstream* f, int64_t a) {
    memcpy(buf_write_int64, &a, 8);
    f->write(buf_write_int64, 8);
}

int64_t WriteCol(std::fstream* file, Column&& c) {
    int64_t res = 0;
    std::visit(Overloaded{
                   [&res, &file](std::vector<int64_t> v) {
                       for (int64_t i : v) {
                           WriteInt64(file, i);
                           res += 8;
                       }
                   },
                   [&res, &file](std::vector<std::string> v) {
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

BZNWriter::BZNWriter(Schema sch, std::fstream* file)
    : schema_(sch), ma_format_(file), locked_(false) {
    ma_format_->seekp(8);  // 8 bytes for meta
}

void BZNWriter::Write(Batch b) {
    DLOG(INFO) << "Writer take batch";
    if (locked_) {
        DLOG(ERROR) << "Write batches closed";
        throw std::exception();
    }

    b.NewSchema(schema_);
    DLOG(INFO) << "Success new scheme";
    offsets_.emplace_back(ma_format_->tellp());
    ma_format_->seekp((sizeof(int64_t)) * b.GetCntColumns(),
                      std::ios::cur);
    std::vector<int64_t> batch_offset;
    for (size_t i = 0; i < b.GetCntColumns(); ++i) {
        batch_offset.emplace_back(ma_format_->tellp());
        WriteCol(ma_format_, std::move(b.GetColumnIdx(i)));
    }

    ma_format_->seekp(offsets_.back(), std::ios::beg);
    WriteCol(ma_format_, Column(std::move(batch_offset)));

    ma_format_->seekp(0, std::ios::end);
}

void BZNWriter::WriteMetaInfo() {
    if (locked_) {
        DLOG(ERROR) << "Already wrote meta";
        throw std::exception();
    }
    locked_ = true;
    int64_t metasize = 0;
    metasize += WriteCol(ma_format_, Column(schema_.GetNames())) + 1;
    ma_format_->put(kMetaDelimiter);

    std::vector<std::string> t;
    for (auto i : schema_.GetTypes()) {
        t.emplace_back(TypeToString(i));
    }
    metasize += WriteCol(ma_format_, Column(std::move(t))) + 1;
    ma_format_->put(kMetaDelimiter);

    DLOG_FIRST_N(INFO, 1) << offsets_.size();
    metasize += WriteCol(ma_format_, Column(std::move(offsets_)));

    ma_format_->seekp(0, std::ios::beg);
    char buf[8];
    memcpy(buf, &metasize, sizeof(metasize));
    ma_format_->write(buf, sizeof(int64_t));
}
