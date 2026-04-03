#include "batch.h"
#include "csv-rw.h"
#include "my-format.h"
#include "operators.h"
#include "expressions.h"
#include "types.h"
#include <fstream>
#include <memory>

namespace exe {
using OperPtr = std::shared_ptr<IOperator>;
using ExprPtr = std::shared_ptr<IExpression>;
};  // namespace exe

void Execute1() {
    std::fstream maformat_file("data.bzn",
                               std::ios::out | std::ios::in | std::ios::binary);
    BZNReader reader(&maformat_file);

    exe::OperPtr scan = std::make_shared<ScanOperator>(
        ScanOperator(BZNReader(&maformat_file), {"AdvEngineID"}));
    exe::ExprPtr AdvEngineRef =
        std::make_shared<ColumnRef>(ColumnRef("AdvEngineID"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::Count}, {AdvEngineRef}).Next();
    std::fstream res_file("01_ans.csv", std::ios::out | std::ios::in |
                                            std::ios::trunc | std::ios::binary);

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}

void Execute2() {
}
void Execute3() {
}
void Execute4() {
}
void Execute5() {
}
void Execute6() {
}
void Execute7() {
}

int main(int, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    Execute1();
    return 0;
}
