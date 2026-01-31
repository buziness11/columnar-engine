#include "csv-rw.h"
#include "my-format.h"
#include <gtest/gtest.h>
#include <cstdint>
#include "batch.h"
#include "schema.h"
#include "types.h"

std::string data_csv_way = TEST_DATA_DIR + std::string("data.csv");
std::string schema_csv_way = TEST_DATA_DIR + std::string("schema.csv");

TEST(BatchWorks, BasicWork) {
    // CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3);
    google::InitGoogleLogging("");
    FLAGS_logtostderr = 1;

    DLOG(INFO) << "Get schema";
    std::fstream schema_file(schema_csv_way,
                             std::ios::in | std::ios::binary);
    Schema schema(&schema_file);

    DLOG(INFO) << "Make reader";
    std::fstream table(data_csv_way, std::ios::in | std::ios::binary);
    CSVReader csv_r(&table, 4);

    DLOG(INFO) << "Make my format";
    std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |
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
                                            std::ios::trunc |
                                            std::ios::binary);

    DLOG(INFO) << "My format to csv";
    CSVWriter csv_w(&res_file);
    while (!bzn_r.IsReaded()) {
        csv_w.WriteBatch(bzn_r.Read());
    }
    // std::vector<std::vector<std::string>> expected{
    //     {"1", "2", "first", "4"},
    //     {"5", "1", "second", "2"},
    //     {"8", "17", "third", "2"}};
    // while (!cr.IsRead()) {
    // }
    // ASSERT_TRUE(expected == actual);
    // expected = {"1", "igor", "19"};
    // actual = cr.GetRow();
    // ASSERT_TRUE(expected == actual);
    // ASSERT_TRUE(cr.IsRead());
    // EXPECT_ANY_THROW(cr.GetRow());
}
