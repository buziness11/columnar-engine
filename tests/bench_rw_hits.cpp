#include "csv-rw.h"
#include "my-format.h"
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <optional>
#include "batch.h"
#include "schema.h"
#include "types.h"

std::string hits_sample_csv_way =
    TEST_DATA_DIR + std::string("hits_sample.csv");
std::string hits_scheme_csv_way =
    TEST_DATA_DIR + std::string("hits_scheme.csv");

TEST(Hits, HitsSample) {
    DLOG(INFO) << "Get schema csv";
    std::fstream schema_file(hits_scheme_csv_way,
                             std::ios::in | std::ios::binary);
    Schema schema(&schema_file);

    DLOG(INFO) << "Make reader";
    std::fstream table(hits_sample_csv_way, std::ios::in | std::ios::binary);
    CSVReader csv_r(&table, schema.GetCntColumns());

    DLOG(INFO) << "Make my format";
    std::fstream maformat_file("hits_sample.bzn", std::ios::out | std::ios::in |
                                                      std::ios::trunc |
                                                      std::ios::binary);
    BZNWriter bzn_w(schema, &maformat_file);

    DLOG(INFO) << "Write my format";
    while (!csv_r.IsReaded()) {
        bzn_w.Write(csv_r.GetBatch());
    }
    bzn_w.WriteMetaInfo();

    DLOG(INFO) << "Build bzn reader my format";
    BZNReader bzn_r(&maformat_file);

    std::fstream res_file("output.csv", std::ios::out | std::ios::in |
                                            std::ios::trunc | std::ios::binary);

    // DLOG(INFO) << "My format to csv";
    CSVWriter csv_w(&res_file);
    while (!bzn_r.IsReaded()) {
        csv_w.WriteBatch(bzn_r.Read());
    }
}
