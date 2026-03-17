#include "batch.h"
#include "csv-rw.h"
#include "schema.h"
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>

// TODO ADD TEST FOR OTHER DELIMITERS

std::string hits_sample_csv_way =
    TEST_DATA_DIR + std::string("hits_sample.csv");
std::string hits_scheme_csv_way =
    TEST_DATA_DIR + std::string("hits_scheme.csv");

std::string basic_test_csv_way_crlf =
    TEST_DATA_DIR + std::string("basic_test.csv");
std::string basic_scheme_csv_way =
    TEST_DATA_DIR + std::string("basic_scheme.csv");

class CsvReaderTest : public ::testing::Test {
protected:
    std::unique_ptr<std::fstream> test_stream;
    std::string temp_filename;

    std::fstream* CreateTestStream(const std::string& content) {
        temp_filename = "temp.csv";
        test_stream = std::make_unique<std::fstream>(
            temp_filename,
            std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        test_stream->write(content.c_str(), content.size());
        test_stream->flush();
        test_stream->seekg(0, std::ios::beg);
        return test_stream.get();
    }

    void TearDown() override {
        test_stream.reset();
    }
};

TEST_F(CsvReaderTest, BasicWork) {
    CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3, ',', false);
    std::vector<std::string> expected{"0", "german", "20"};
    std::vector<std::string> actual = cr.GetRow();
    ASSERT_TRUE(expected == actual);
    expected = {"1", "igor", "19"};
    actual = cr.GetRow();
    ASSERT_TRUE(expected == actual);
    ASSERT_TRUE(cr.IsReaded());
    EXPECT_ANY_THROW(cr.GetRow());
}

TEST_F(CsvReaderTest, CRLF) {
    std::vector<std::pair<std::string, bool>> tables_crlf_lf = {
        {"0,german,20\r\n1,igor,19", false},
        {"0,german,20\r\n1,igor,19\r\n", false}};
    for (auto [table, lf] : tables_crlf_lf) {
        CSVReader cr(CreateTestStream(table), 3, ',', lf);
        std::vector<std::string> expected{"0", "german", "20"};
        std::vector<std::string> actual = cr.GetRow();
        ASSERT_TRUE(expected == actual);
        expected = {"1", "igor", "19"};
        actual = cr.GetRow();
        ASSERT_TRUE(expected == actual);
        ASSERT_TRUE(cr.IsReaded());
        EXPECT_ANY_THROW(cr.GetRow());
    }
}

TEST_F(CsvReaderTest, LF) {
    std::vector<std::pair<std::string, bool>> tables_crlf_lf = {
        {"0,german,20\n1,igor,19", true}, {"0,german,20\n1,igor,19\n", true}};
    for (auto [table, lf] : tables_crlf_lf) {
        CSVReader cr(CreateTestStream(table), 3, ',', lf);
        std::vector<std::string> expected{"0", "german", "20"};
        std::vector<std::string> actual = cr.GetRow();
        ASSERT_TRUE(expected == actual);
        expected = {"1", "igor", "19"};
        actual = cr.GetRow();
        ASSERT_TRUE(expected == actual);
        ASSERT_TRUE(cr.IsReaded());
        EXPECT_ANY_THROW(cr.GetRow());
    }
}

TEST_F(CsvReaderTest, EmptyFields) {
    std::vector<std::string> emptys = {",,", ",,\n"};
    for (auto s : emptys) {
        CSVReader cr(CreateTestStream(",,\n"), 3);
        auto row = cr.GetRow();

        for (auto i : row) {
            std::cerr << "now: \"" << i << "\"\n";
        }
        EXPECT_EQ(row.size(), 3);
        ASSERT_TRUE(row == std::vector<std::string>(3, ""));
        ASSERT_TRUE(cr.IsReaded());
    }
}

TEST_F(CsvReaderTest, NeqCntInColumns) {
    CSVReader cr(CreateTestStream(",\r\n,,"), 2);
    cr.GetRow();
    EXPECT_ANY_THROW(cr.GetRow());
}

TEST_F(CsvReaderTest, ZeroColumns) {
    CSVReader cr(CreateTestStream(",,,"), 0);
    EXPECT_ANY_THROW(cr.GetRow());
}

TEST_F(CsvReaderTest, QuotesNorm) {
    std::vector<std::pair<std::string, uint8_t>> couldnotbebroken{
        {"\"\"", 1},           // ""
        {"\"\"\",\"\"\"", 1},  // " "" , "" "
        {"\"\r\n,\r\"", 1}     // "\r\n,\r"
    };
    for (auto [s, c] : couldnotbebroken) {
        std::cerr << s << '\n';
        CSVReader cr(CreateTestStream(s), c);
        while (!cr.IsReaded()) {
            cr.GetRow();
        }
    }
}

TEST_F(CsvReaderTest, QuotesBroken) {
    std::vector<std::pair<std::string, uint8_t>> couldbebroken{
        {"\"", 1},      // "
        {"\"\"\"", 1},  // """
        {"\",1,2,3,", 1}};

    for (auto [s, c] : couldbebroken) {
        CSVReader cr(CreateTestStream(s), c);
        EXPECT_ANY_THROW(cr.GetRow());
    }
}

TEST_F(CsvReaderTest, NormalFileCRLF) {
    std::string way = basic_test_csv_way_crlf;
    std::fstream file(way, std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "cant open file";
    std::vector<std::vector<std::string>> expected{{"1", "2", "first", "4"},
                                                   {"5", "1", "second", "2"},
                                                   {"8", "17", "third", "2"}};
    CSVReader cr(&file, 4, ',', false);
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_TRUE(cr.GetRow() == expected[i]);
    }
    ASSERT_TRUE(cr.IsReaded());
}

TEST_F(CsvReaderTest, Batch) {
    std::string way = basic_test_csv_way_crlf;
    std::vector<std::vector<std::string>> expected{
        {"1", "5", "8"},
        {"2", "1", "17"},
        {"first", "second", "third"},
        {"4", "2", "2"},
    };
    std::fstream file(way, std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "cant open file";
    CSVReader cr(&file, 4);
    Batch b = cr.GetBatch();
    for (size_t i = 0; i < 4; ++i) {
        std::visit(Overloaded{[&](std::vector<std::string> v) {
                                  ASSERT_EQ(v.size(), expected[i].size());
                              },
                              [](auto&&) {
                                  ASSERT_TRUE(false) << "Wrong csv backet type";
                              }},
                   b.GetColumnIdx(i).GetData());
    }
}

const size_t kHitsCntColumns = 105;
TEST_F(CsvReaderTest, Hits) {
    std::fstream schema_file(hits_scheme_csv_way,
                             std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(schema_file.is_open()) << "cant open file";
    Schema schema_hits(&schema_file);
    ASSERT_EQ(schema_hits.GetCntColumns(), kHitsCntColumns);
    {
        std::vector<std::string> name_check{"WatchID",
                                            "JavaEnable",
                                            "Title",
                                            "GoodEvent",
                                            "EventTime",
                                            "EventDate",
                                            "CounterID",
                                            "ClientIP",
                                            "RegionID",
                                            "UserID",
                                            "CounterClass",
                                            "OS",
                                            "UserAgent",
                                            "URL",
                                            "Referer",
                                            "IsRefresh",
                                            "RefererCategoryID",
                                            "RefererRegionID",
                                            "URLCategoryID",
                                            "URLRegionID",
                                            "ResolutionWidth",
                                            "ResolutionHeight",
                                            "ResolutionDepth",
                                            "FlashMajor",
                                            "FlashMinor",
                                            "FlashMinor2",
                                            "NetMajor",
                                            "NetMinor",
                                            "UserAgentMajor",
                                            "UserAgentMinor",
                                            "CookieEnable",
                                            "JavascriptEnable",
                                            "IsMobile",
                                            "MobilePhone",
                                            "MobilePhoneModel",
                                            "Params",
                                            "IPNetworkID",
                                            "TraficSourceID",
                                            "SearchEngineID",
                                            "SearchPhrase",
                                            "AdvEngineID",
                                            "IsArtifical",
                                            "WindowClientWidth",
                                            "WindowClientHeight",
                                            "ClientTimeZone",
                                            "ClientEventTime",
                                            "SilverlightVersion1",
                                            "SilverlightVersion2",
                                            "SilverlightVersion3",
                                            "SilverlightVersion4",
                                            "PageCharset",
                                            "CodeVersion",
                                            "IsLink",
                                            "IsDownload",
                                            "IsNotBounce",
                                            "FUniqID",
                                            "OriginalURL",
                                            "HID",
                                            "IsOldCounter",
                                            "IsEvent",
                                            "IsParameter",
                                            "DontCountHits",
                                            "WithHash",
                                            "HitColor",
                                            "LocalEventTime",
                                            "Age",
                                            "Sex",
                                            "Income",
                                            "Interests",
                                            "Robotness",
                                            "RemoteIP",
                                            "WindowName",
                                            "OpenerName",
                                            "HistoryLength",
                                            "BrowserLanguage",
                                            "BrowserCountry",
                                            "SocialNetwork",
                                            "SocialAction",
                                            "HTTPError",
                                            "SendTiming",
                                            "DNSTiming",
                                            "ConnectTiming",
                                            "ResponseStartTiming",
                                            "ResponseEndTiming",
                                            "FetchTiming",
                                            "SocialSourceNetworkID",
                                            "SocialSourcePage",
                                            "ParamPrice",
                                            "ParamOrderID",
                                            "ParamCurrency",
                                            "ParamCurrencyID",
                                            "OpenstatServiceName",
                                            "OpenstatCampaignID",
                                            "OpenstatAdID",
                                            "OpenstatSourceID",
                                            "UTMSource",
                                            "UTMMedium",
                                            "UTMCampaign",
                                            "UTMContent",
                                            "UTMTerm",
                                            "FromTag",
                                            "HasGCLID",
                                            "RefererHash",
                                            "URLHash",
                                            "CLID"};
        std::vector<std::string> names = schema_hits.GetNames();
        for (size_t i = 0; i < kHitsCntColumns; ++i) {
            ASSERT_EQ(names[i], name_check[i]);
        }
    }

    std::fstream hits_file(hits_sample_csv_way,
                           std::ios::in | std::ios::out | std::ios::binary);
}
