#include "src/csv-rw.h"
#include "src/my-format.h"

std::string hits_small_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_sample.csv");
std::string hits_scheme_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_scheme.csv");

int main() {
    std::fstream schema_file(hits_scheme_csv_way_lf,
                             std::ios::in | std::ios::binary);
    Schema schema(&schema_file);
    std::fstream table(hits_small_csv_way_lf, std::ios::in | std::ios::binary);
    CSVReader csv_r(&table, schema.GetCntColumns());
    std::fstream maformat_file("data.bzn", std::ios::out | std::ios::in |
                                               std::ios::trunc |
                                               std::ios::binary);
    BZNWriter bzn_w(schema, &maformat_file);
    while (!csv_r.IsReaded()) {
        bzn_w.Write(csv_r.GetBatch());
    }
    bzn_w.WriteMetaInfo();
}
