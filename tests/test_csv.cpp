#include "csv-reader.h"
#include <gtest/gtest.h>
#include <cstdint>

// TODO ADD TEST FOR OTHER DELIMITERS

class CsvReaderTest : public ::testing::Test {
protected:
    std::unique_ptr<std::istream> test_stream;

    std::istream* CreateTestStream(const std::string& content) {
        test_stream = std::make_unique<std::stringstream>(content);
        return test_stream.get();
    }

    void TearDown() override {
        test_stream.reset();
    }
};

TEST_F(CsvReaderTest, BasicWork) {
    CSVReader cr(CreateTestStream("0,german,20\r\n1,igor,19"), 3);
    std::vector<std::string> expected{"0", "german", "20"};
    std::vector<std::string> actual = cr.GetRow();
    ASSERT_TRUE(expected == actual);
    expected = {"1", "igor", "19"};
    actual = cr.GetRow();
    ASSERT_TRUE(expected == actual);
    ASSERT_TRUE(cr.IsRead());
    EXPECT_ANY_THROW(cr.GetRow());
}

TEST_F(CsvReaderTest, EmptyFields) {
    CSVReader cr(CreateTestStream(",,"), 3);
    auto row = cr.GetRow();

    EXPECT_EQ(row.size(), 3);
    ASSERT_TRUE(row == std::vector<std::string>(3, ""));
    ASSERT_TRUE(cr.IsRead());
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
        while (!cr.IsRead()) {
            cr.GetRow();
        }
    }
}

TEST_F(CsvReaderTest, QuotesBroken) {
    std::vector<std::pair<std::string, uint8_t>> couldbebroken{{"\"", 1},      // "
                                                               {"\"\"\"", 1},  // """
                                                               {"\",1,2,3,", 1}};

    for (auto [s, c] : couldbebroken) {
        CSVReader cr(CreateTestStream(s), c);
        EXPECT_ANY_THROW(cr.GetRow());
    }
}
