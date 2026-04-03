#include "csv-rw.h"
#include "my-format.h"
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include "batch.h"
#include "schema.h"
#include "types.h"

std::string basic_test_csv_way_crlf =
    TEST_DATA_DIR + std::string("basic_test_crlf.csv");
std::string basic_scheme_csv_way_crlf =
    TEST_DATA_DIR + std::string("basic_scheme_crlf.csv");

TEST(BatchWorks, BasicWorkCRLF) {
    // CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3);
    FLAGS_logtostderr = 1;
    std::vector<std::optional<int>> batch_sizes{std::nullopt, 1, 2, 3};
    std::vector<std::vector<std::string>> expected{{"1", "2", "first", "4"},
                                                   {"5", "1", "second", "2"},
                                                   {"8", "17", "third", "2"}};
    for (auto batch_size : batch_sizes) {
        DLOG(INFO) << "Get schema";
        std::fstream schema_file(basic_scheme_csv_way_crlf,
                                 std::ios::in | std::ios::binary);
        Schema schema(&schema_file, ',', false);  // basic_scheme crlf

        DLOG(INFO) << "Make reader";
        std::fstream table(basic_test_csv_way_crlf,
                           std::ios::in | std::ios::binary);
        CSVReader csv_r(&table, 4, ',', false);  // basic_table crlf

        DLOG(INFO) << "Make my format";
        std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |
                                                   std::ios::trunc |
                                                   std::ios::binary);
        BZNWriter bzn_w(schema, &maformat_file);

        DLOG(INFO) << "Write my format";
        while (!csv_r.IsReaded()) {
            if (!batch_size) {
                bzn_w.Write(csv_r.GetBatch());
            } else {
                bzn_w.Write(csv_r.GetBatch(*batch_size));
            }
        }
        bzn_w.WriteMetaInfo();

        DLOG(INFO) << "Build bzn reader my format";
        BZNReader bzn_r(&maformat_file);

        std::fstream res_file("output.csv", std::ios::out | std::ios::in |
                                                std::ios::trunc |
                                                std::ios::binary);

        DLOG(INFO) << "My format to csv";
        CSVWriter csv_w(&res_file, false);
        while (!bzn_r.IsReaded()) {
            csv_w.WriteBatch(bzn_r.Read());
        }

        std::fstream outfile("output.csv", std::ios::in | std::ios::binary);
        CSVReader out_r(&outfile, 4, ',', false);
        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(out_r.GetRow(), expected[i]);
        }
        ASSERT_TRUE(out_r.IsReaded());
    }
}

std::string basic_test_csv_way_lf =
    TEST_DATA_DIR + std::string("basic_test_lf.csv");
std::string basic_scheme_csv_way_lf =
    TEST_DATA_DIR + std::string("basic_scheme_lf.csv");

TEST(BatchWorks, BasicWorkLF) {
    // CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3);
    FLAGS_logtostderr = 1;
    std::vector<std::optional<int>> batch_sizes{std::nullopt, 1, 2, 3};
    std::vector<std::vector<std::string>> expected{{"1", "2", "first", "4"},
                                                   {"5", "1", "second", "2"},
                                                   {"8", "17", "third", "2"}};
    for (auto batch_size : batch_sizes) {
        // DLOG(INFO) << "Get schema";
        std::fstream schema_file(basic_scheme_csv_way_lf,
                                 std::ios::in | std::ios::binary);
        Schema schema(&schema_file);

        // DLOG(INFO) << "Make reader";
        std::fstream table(basic_test_csv_way_lf,
                           std::ios::in | std::ios::binary);
        CSVReader csv_r(&table, 4);

        // DLOG(INFO) << "Make my format";
        std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |
                                                   std::ios::trunc |
                                                   std::ios::binary);
        BZNWriter bzn_w(schema, &maformat_file);

        // DLOG(INFO) << "Write my format";
        while (!csv_r.IsReaded()) {
            if (!batch_size) {
                bzn_w.Write(csv_r.GetBatch());
            } else {
                bzn_w.Write(csv_r.GetBatch(*batch_size));
            }
        }
        bzn_w.WriteMetaInfo();

        // DLOG(INFO) << "Build bzn reader my format";
        BZNReader bzn_r(&maformat_file);

        std::fstream res_file("output.csv", std::ios::out | std::ios::in |
                                                std::ios::trunc |
                                                std::ios::binary);

        // DLOG(INFO) << "My format to csv";
        CSVWriter csv_w(&res_file);
        while (!bzn_r.IsReaded()) {
            csv_w.WriteBatch(bzn_r.Read());
        }

        std::fstream outfile("output.csv", std::ios::in | std::ios::binary);
        CSVReader out_r(&outfile, 4);
        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(out_r.GetRow(), expected[i]);
        }
        ASSERT_TRUE(out_r.IsReaded());
    }
}

TEST(BZNReader, CheckReadByNames) {

    std::fstream schema_file(basic_scheme_csv_way_lf,
                             std::ios::in | std::ios::binary);
    Schema schema(&schema_file);

    DLOG(INFO) << "Make my format";
    std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |

                                               std::ios::binary);

    // DLOG(INFO) << "Build bzn reader my format";
    BZNReader bzn_r(&maformat_file);

    std::fstream res_file("output.csv", std::ios::out | std::ios::in |
                                            std::ios::trunc | std::ios::binary);

    DLOG(INFO) << "My format to csv";
    CSVWriter csv_w(&res_file);
    while (!bzn_r.IsReaded()) {
        DLOG(INFO) << "write";
        csv_w.WriteBatch(bzn_r.Read({"name123", "a"}));
    }

    std::vector<std::vector<std::string>> expected{
        {"1", "first"}, {"5", "second"}, {"8", "third"}};

    std::fstream outfile("output.csv", std::ios::in | std::ios::binary);
    CSVReader out_r(&outfile, 2);
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(out_r.GetRow(), expected[i]);
    }
    ASSERT_TRUE(out_r.IsReaded());
}

std::string hits_small_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_small_sample.csv");
std::string hits_scheme_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_scheme.csv");

const size_t kBatchSmallSize = 201;

bool EqStringPredicate(std::string s1, std::string s2) {
    if (s1 == s2) {
        return true;
    }
    s1 = "\"" + s1 + "\"";
    return s1 == s2;
}

TEST(Hits, HitsSmallGood) {
    // CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3);
    std::fstream schema_file(hits_scheme_csv_way_lf,
                             std::ios::in | std::ios::binary);
    Schema schema(&schema_file);

    // DLOG(INFO) << "Make reader";
    std::fstream table(hits_small_csv_way_lf, std::ios::in | std::ios::binary);
    CSVReader csv_r(&table, schema.GetCntColumns());

    // DLOG(INFO) << "Make my format";
    std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |
                                               std::ios::trunc |
                                               std::ios::binary);
    BZNWriter bzn_w(schema, &maformat_file);

    // DLOG(INFO) << "Write my format";
    while (!csv_r.IsReaded()) {
        bzn_w.Write(csv_r.GetBatch(kBatchSmallSize));
    }
    bzn_w.WriteMetaInfo();

    // DLOG(INFO) << "Build bzn reader my format";
    BZNReader bzn_r(&maformat_file);

    std::fstream res_file("output.csv", std::ios::out | std::ios::in |
                                            std::ios::trunc | std::ios::binary);

    // DLOG(INFO) << "My format to csv";
    CSVWriter csv_w(&res_file);
    while (!bzn_r.IsReaded()) {
        csv_w.WriteBatch(bzn_r.Read());
    }

    std::fstream outfile("output.csv", std::ios::in | std::ios::binary);
    CSVReader out_r(&outfile, schema.GetCntColumns());

    std::fstream expected(hits_small_csv_way_lf,
                          std::ios::in | std::ios::binary);
    CSVReader out_expected(&expected, schema.GetCntColumns());

    while (!out_expected.IsReaded()) {
        std::vector<std::string> expected_strs = out_expected.GetRow();
        std::vector<std::string> out_strs = out_r.GetRow();
        ASSERT_EQ(expected_strs.size(), out_strs.size());
        for (size_t i = 0; i < out_strs.size(); ++i) {
            ASSERT_PRED2(EqStringPredicate, out_strs[i], expected_strs[i]);
        }
    }
    ASSERT_TRUE(out_r.IsReaded() && out_expected.IsReaded());
}

TEST(Types, TypesTranslation) {
    // CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3);
    std::vector<Types> tps = {Types::kInt16_t, Types::kInt32_t,
                              Types::kInt64_t, Types::kString,
                              Types::kDate,    Types::kTimestamp};
    std::vector<std::string> strs = {"int16",  "int32", "int64",
                                     "string", "DATE",  "TIMESTAMP"};
    for (size_t i = 0; i < tps.size(); ++i) {
        ASSERT_EQ(tps[i], StringToType(strs[i]));
        ASSERT_EQ(strs[i], TypeToString(StringToType(strs[i])));
        ASSERT_EQ(strs[i], TypeToString(tps[i]));
        ASSERT_EQ(tps[i], StringToType(TypeToString(tps[i])));
    }
}
