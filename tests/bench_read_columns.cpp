#include "core/batch.h"
#include "core/schema.h"
#include "core/types.h"
#include "io/csv-rw.h"
#include "io/my-format.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

const std::string kHitsSampleCsv =
    TEST_DATA_DIR + std::string("hits_sample.csv");
const std::string kHitsSchemeCsv =
    TEST_DATA_DIR + std::string("hits_scheme.csv");
const std::string kHitsSampleBzn = "hits_sample.bzn";
constexpr size_t kCachePollutionSize = 512ull * 1024ull * 1024ull;

Schema LoadSchema() {
    std::fstream schema_file(kHitsSchemeCsv, std::ios::in | std::ios::binary);
    if (!schema_file.is_open()) {
        throw std::runtime_error("Cannot open schema file: " + kHitsSchemeCsv);
    }
    return Schema(&schema_file);
}

void EnsureBznExists(const Schema& schema) {
    {
        std::fstream existing(kHitsSampleBzn, std::ios::in | std::ios::binary);
        if (existing.good()) {
            return;
        }
    }

    std::fstream table(kHitsSampleCsv, std::ios::in | std::ios::binary);
    if (!table.is_open()) {
        throw std::runtime_error("Cannot open CSV file: " + kHitsSampleCsv);
    }

    CsvReader csv_reader(&table, schema.GetCntColumns());
    std::fstream bzn_file(
        kHitsSampleBzn,
        std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
    if (!bzn_file.is_open()) {
        throw std::runtime_error("Cannot create BZN file: " + kHitsSampleBzn);
    }

    BZNWriter writer(schema, &bzn_file);
    while (!csv_reader.IsReaded()) {
        writer.Write(csv_reader.GetBatch());
    }
    writer.WriteMetaInfo();
}

std::vector<std::string> FirstColumns(const Schema& schema, size_t count) {
    const std::vector<std::string>& names = schema.GetNames();
    count = std::min(count, names.size());
    return std::vector<std::string>(names.begin(), names.begin() + count);
}

std::vector<std::string> ColumnsByType(const Schema& schema, Types type) {
    std::vector<std::string> result;
    const std::vector<std::string>& names = schema.GetNames();
    const std::vector<Types>& types = schema.GetTypes();
    for (size_t i = 0; i < names.size(); ++i) {
        if (types[i] == type) {
            result.emplace_back(names[i]);
        }
    }
    return result;
}

size_t ReadAllBatches(const std::vector<std::string>& columns) {
    std::fstream bzn_file(kHitsSampleBzn, std::ios::in | std::ios::binary);
    if (!bzn_file.is_open()) {
        throw std::runtime_error("Cannot open BZN file: " + kHitsSampleBzn);
    }

    BZNReader reader(&bzn_file);
    size_t rows = 0;
    while (!reader.IsReaded()) {
        Batch batch = reader.Read(columns);
        rows += batch.GetColumnSize();
    }
    return rows;
}

void PolluteCache() {
    static std::vector<std::byte> buffer(kCachePollutionSize);
    std::byte acc{0};
    for (size_t i = 0; i < buffer.size(); i += 64) {
        buffer[i] = std::byte{static_cast<unsigned char>(i)};
        acc ^= buffer[i];
    }
    benchmark::DoNotOptimize(acc);
    benchmark::ClobberMemory();
}

class BznReadBenchmark : public benchmark::Fixture {
public:
    static Schema& GetSchema() {
        static Schema schema = LoadSchema();
        return schema;
    }

    void SetUp(const benchmark::State&) override {
        static const bool prepared = []() {
            EnsureBznExists(GetSchema());
            return true;
        }();
        if (!prepared) {
            throw std::runtime_error("benchmark preparation failed");
        }
    }
};

void RunReadBenchmark(benchmark::State& state,
                      const std::vector<std::string>& columns,
                      std::string_view label) {
    size_t rows = 0;
    for (auto _ : state) {
        state.PauseTiming();
        PolluteCache();
        state.ResumeTiming();

        rows = ReadAllBatches(columns);
        benchmark::DoNotOptimize(rows);
    }

    state.counters["columns"] = static_cast<double>(
        columns.empty() ? BznReadBenchmark::GetSchema().GetCntColumns()
                        : columns.size());
    state.counters["rows"] = static_cast<double>(rows);
    state.counters["rows_per_second"] =
        benchmark::Counter(static_cast<double>(rows),
                           benchmark::Counter::kIsRate);
    state.SetLabel(std::string(label));
}

BENCHMARK_DEFINE_F(BznReadBenchmark, ByColumnCount)(benchmark::State& state) {
    const Schema& schema = GetSchema();
    const size_t requested = static_cast<size_t>(state.range(0));
    std::vector<std::string> columns;
    std::string label;
    if (requested == schema.GetCntColumns()) {
        label = "all";
    } else {
        columns = FirstColumns(schema, requested);
        label = std::to_string(requested);
    }
    RunReadBenchmark(state, columns, label);
}

BENCHMARK_DEFINE_F(BznReadBenchmark, ByColumnType)(benchmark::State& state) {
    const Schema& schema = GetSchema();
    const auto type = static_cast<Types>(state.range(0));
    std::vector<std::string> columns = ColumnsByType(schema, type);
    RunReadBenchmark(state, columns, TypeToString(type));
}

BENCHMARK_REGISTER_F(BznReadBenchmark, ByColumnCount)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->Arg(16)
    ->Arg(32)
    ->Arg(64)
    ->Arg(105)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(5);

BENCHMARK_REGISTER_F(BznReadBenchmark, ByColumnType)
    ->Arg(static_cast<int64_t>(Types::kInt16_t))
    ->Arg(static_cast<int64_t>(Types::kInt32_t))
    ->Arg(static_cast<int64_t>(Types::kInt64_t))
    ->Arg(static_cast<int64_t>(Types::kString))
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(5);

}  // namespace

BENCHMARK_MAIN();
