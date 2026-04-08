#include <cstddef>
#include <functional>
#include <string>
#include "batch.h"
#include "csv-rw.h"
#include "my-format.h"
#include "operators.h"
#include "expressions.h"
#include "types.h"
#if defined(__APPLE__)
#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED  // macos only
#endif
#include <boost/stacktrace.hpp>
#include <csignal>
#include <fstream>
#include <memory>

namespace exe {
using OperPtr = std::shared_ptr<IOperator>;
using ExprPtr = std::shared_ptr<IExpression>;
};  // namespace exe

std::fstream maformat_file;
std::fstream res_file;

exe::OperPtr GetScanner(std::vector<std::string> names) {
    maformat_file.seekg(0, std::ios::beg);
    maformat_file.seekp(0, std::ios::beg);
    BZNReader reader(&maformat_file);

    exe::OperPtr scan = std::make_shared<ScanOperator>(
        ScanOperator(BZNReader(&maformat_file), std::move(names)));
    return scan;
}

void Execute0() {
    DLOG(INFO) << "0th que";
    exe::OperPtr scan = GetScanner({"AdvEngineID"});
    exe::ExprPtr AdvEngineRef =
        std::make_shared<ColumnRef>(ColumnRef("AdvEngineID"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::Count}, {AdvEngineRef}).Next();

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}

void Execute1() {
    DLOG(INFO) << "1st que";
    exe::OperPtr scan = GetScanner({"AdvEngineID"});
    exe::ExprPtr AdvEngineID =
        std::make_shared<ColumnRef>(ColumnRef("AdvEngineID"));
    exe::ExprPtr ZeroLiteral =
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(AdvEngineID, CmpType::Neq, ZeroLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);
    std::optional<Batch> res =
        AggregateOperator(filter, {AggregateType::Count}, {AdvEngineID}).Next();

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute2() {
    DLOG(INFO) << "2nd que";
    exe::OperPtr scan = GetScanner({"AdvEngineID", "ResolutionWidth"});
    exe::ExprPtr AdvEngineID =
        std::make_shared<ColumnRef>(ColumnRef("AdvEngineID"));
    exe::ExprPtr ResolutionWidth =
        std::make_shared<ColumnRef>(ColumnRef("ResolutionWidth"));
    std::vector<AggregateType> aggs = {AggregateType::Sum, AggregateType::Count,
                                       AggregateType::Avg};
    std::vector<exe::ExprPtr> exprs = {AdvEngineID, AdvEngineID,
                                       ResolutionWidth};
    exe::OperPtr agg_op =
        std::make_shared<AggregateOperator>(scan, aggs, exprs);

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*(agg_op->Next()));
}

void Execute3() {
    DLOG(INFO) << "3rd que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>(ColumnRef("UserID"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::Avg}, {UserID}).Next();

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute4() {
    DLOG(INFO) << "4th que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>(ColumnRef("UserID"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::CountDistinct}, {UserID})
            .Next();

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute5() {
    DLOG(INFO) << "5th que";
    exe::OperPtr scan = GetScanner({"SearchPhrase"});
    exe::ExprPtr UserID =
        std::make_shared<ColumnRef>(ColumnRef("SearchPhrase"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::CountDistinct}, {UserID})
            .Next();

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute6() {
    DLOG(INFO) << "6th que";
    exe::OperPtr scan = GetScanner({"EventDate"});
    exe::ExprPtr EventDate =
        std::make_shared<ColumnRef>(ColumnRef("EventDate"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::Min, AggregateType::Max}, {EventDate, EventDate})
            .Next();

    CSVWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute7() {
}

void Execute8() {
}
void Execute9() {
}

void Execute10() {
}
void Execute11() {
}

void Execute12() {
}
void Execute13() {
}

void Execute14() {
}
void Execute15() {
}

void Execute16() {
}
void Execute17() {
}

void Execute18() {
}
void Execute19() {
}

void Execute20() {
}
void Execute21() {
}

void Execute22() {
}
void Execute23() {
}

void my_handler(int signum) {  // gemini handler
    DLOG(INFO) << boost::stacktrace::stacktrace();
    std::exit(signum);
}

std::vector<std::function<void()>> executors = {
    Execute0,  Execute1,  Execute2,  Execute3,  Execute4,  Execute5,
    Execute6,  Execute7,  Execute8,  Execute9,  Execute10, Execute11,
    Execute12, Execute13, Execute14, Execute15, Execute16, Execute17,
    Execute18, Execute19, Execute20, Execute21, Execute22, Execute23};

int main(int, char** argv) {
    // argv[0], que_num, columnar, output, logs
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    for (int sig : {SIGSEGV, SIGABRT, SIGFPE, SIGILL}) {
        std::signal(sig, my_handler);
    }
    maformat_file =
        std::fstream(argv[2], std::ios::out | std::ios::in | std::ios::binary);
    res_file = std::fstream(argv[3], std::ios::out | std::ios::in |
                                         std::ios::trunc | std::ios::binary);
    size_t idx = std::stoi(argv[1]);
    if (idx < executors.size()) {
        executors[idx]();
    } else {
        DLOG(INFO) << "dont support " << idx << " question now can only"
                   << executors.size();
    }
    return 0;
}
