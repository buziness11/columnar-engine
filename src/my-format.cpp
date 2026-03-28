#include "my-format.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <variant>
#include <vector>
#include "batch.h"
#include "column.h"
#include "schema.h"
#include "rwconsts.h"
#include "types.h"
#include <glog/logging.h>

BZNReader::BZNReader(std::fstream* file) : ma_format_(file) {
    ma_format_->seekg(0, std::ios::beg);
    int64_t metasize =
        ReadColFromBzn(ma_format_, Types::kInt64_t, sizeof(metasize))
            .GetElementByIndex<int64_t>(0);
    ma_format_->seekg(-metasize, std::ios::end);

    BuildSchema();

    int64_t temp_offset = ma_format_->tellg();
    ma_format_->seekg(0, std::ios::end);
    int64_t file_end = ma_format_->tellg();
    ma_format_->seekg(temp_offset);
    GetMetaOffset(file_end);

    ma_format_->seekg(-metasize, std::ios::end);
    bzn_file_offsets_.emplace_back(ma_format_->tellg());

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

void BZNReader::GetMetaOffset(int64_t file_end) {
    std::visit(
        Overloaded{[&](std::vector<int64_t>&& v) { bzn_file_offsets_ = v; },
                   [](auto&&) {
                       DLOG(ERROR) << "Wrong read batch offsets";
                       throw std::exception();
                   }},
        ReadColFromBzn(ma_format_, Types::kInt64_t, file_end).GetData());
}

std::vector<std::string> BZNReader::GetMetaString() {
    std::vector<std::string> res;
    while (ma_format_->peek() != kMetaDelimiter) {
        std::string s;
        while (ma_format_->peek() != kStringDelimiter) {
            s.push_back(ma_format_->get());
        }
        ma_format_->get();
        res.emplace_back(std::move(s));
    }
    ma_format_->get();
    return res;
}

std::vector<int64_t> BZNReader::GetMetaBatchOffset() {
    std::vector<int64_t> dataoffset(schema_.GetCntColumns());

    std::visit(Overloaded{[&](std::vector<int64_t>&& v) { dataoffset = v; },
                          [](auto&&) { throw std::exception(); }},
               ReadColFromBzn(ma_format_, Types::kInt64_t,
                              static_cast<int64_t>(ma_format_->tellg()) +
                                  dataoffset.size() * sizeof(int64_t))
                   .GetData());
    return dataoffset;
}

size_t BZNReader::GetCntColumns() {
    return schema_.GetCntColumns();
}

bool BZNReader::IsReaded() {
    return ma_format_->tellg() >= bzn_file_offsets_.back();
}

Batch BZNReader::Read(const std::vector<std::string>& column_peek) {
    if (IsReaded()) {
        DLOG(ERROR) << "all batches readed";
        throw std::exception();
    }
    ma_format_->seekg(bzn_file_offsets_[cur_batch_], std::ios::beg);
    std::vector<int64_t> column_offset = GetMetaBatchOffset();
    column_offset.emplace_back(bzn_file_offsets_[++cur_batch_]);
    std::vector<Column> bat;

    for (size_t i = 0; i < schema_.GetCntColumns(); ++i) {
        if (column_peek.empty() || find(column_peek.begin(), column_peek.end(), schema_.GetNames()[i]) != column_peek.end()) {  // TODO
            bat.emplace_back(ReadColFromBzn(ma_format_, schema_.GetType(i),
                                            column_offset[i + 1]));
        } else {
            ma_format_->seekg(column_offset[i + 1]);
        }
    }
    return Batch(schema_, std::move(bat));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BZNWriter::BZNWriter(Schema sch, std::fstream* file)
    : schema_(sch), ma_format_(file), locked_(false) {
    ma_format_->seekp(8);  // 8 bytes for meta
}

void BZNWriter::Write(Batch b) {
    if (locked_) {
        DLOG(ERROR) << "Write batches closed";
        throw std::exception();
    }

    b.NewSchema(schema_);
    bzn_file_offsets_.emplace_back(ma_format_->tellp());
    ma_format_->seekp((sizeof(int64_t)) * b.GetCntColumns(), std::ios::cur);
    std::vector<int64_t> batch_offset;
    for (size_t i = 0; i < b.GetCntColumns(); ++i) {
        batch_offset.emplace_back(ma_format_->tellp());
        WriteColToBzn(ma_format_, std::move(b.GetColumnIdx(i)));
    }

    ma_format_->seekp(bzn_file_offsets_.back(), std::ios::beg);
    WriteColToBzn(ma_format_, Column(std::move(batch_offset), Types::kInt64_t));

    ma_format_->seekp(0, std::ios::end);
}

void BZNWriter::WriteMetaInfo() {
    if (locked_) {
        DLOG(ERROR) << "Already wrote meta";
        throw std::exception();
    }
    locked_ = true;
    int64_t metasize = 0;
    metasize +=
        WriteColToBzn(ma_format_, Column(schema_.GetNames(), Types::kString)) +
        1;
    ma_format_->put(kMetaDelimiter);

    std::vector<std::string> types;
    for (auto i : schema_.GetTypes()) {
        types.emplace_back(TypeToString(i));
    }
    metasize +=
        WriteColToBzn(ma_format_, Column(std::move(types), Types::kString)) + 1;
    ma_format_->put(kMetaDelimiter);

    metasize += WriteColToBzn(
        ma_format_, Column(std::move(bzn_file_offsets_), Types::kInt64_t));

    ma_format_->seekp(0, std::ios::beg);
    char buf[sizeof(metasize)];
    memcpy(buf, &metasize, sizeof(metasize));
    ma_format_->write(buf, sizeof(int64_t));
}
