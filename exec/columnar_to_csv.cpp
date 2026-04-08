#include "src/csv-rw.h"
#include "src/my-format.h"

std::string hits_small_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_small_sample.csv");
std::string hits_scheme_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_scheme.csv");

int main() {
    std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |
                                               std::ios::trunc |
                                               std::ios::binary);
    BZNReader bzn_r(&maformat_file);

    std::fstream res_file("output.csv", std::ios::out | std::ios::in |
                                            std::ios::trunc | std::ios::binary);

    CSVWriter csv_w(&res_file);
    while (!bzn_r.IsReaded()) {
        csv_w.WriteBatch(bzn_r.Read());
    }
}
