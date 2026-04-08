#include <cstddef>
#include <functional>
#include <optional>
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

/*
15567353102073
1948165676197968384

*/

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
    DLOG(INFO) << "1st que";
    exe::OperPtr scan = GetScanner({"UserID"});
    DLOG(INFO) << "scan getted";
    std::optional<Batch> nw = scan->Next();
    CSVWriter csv_w(&res_file);
    while (nw) {
        DLOG(INFO) << "write";
        csv_w.WriteBatch(*nw);
        DLOG(INFO) << "writen";
        nw = scan->Next();
    }
}

void my_handler(int signum) {  // gemini handler
    DLOG(INFO) << boost::stacktrace::stacktrace();
    std::exit(signum);
}

int main(int, char** argv) {
    // argv[0], columnar, output, logs
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    for (int sig : {SIGSEGV, SIGABRT, SIGFPE, SIGILL}) {
        std::signal(sig, my_handler);
    }
    maformat_file =
        std::fstream(argv[1], std::ios::out | std::ios::in | std::ios::binary);
    res_file = std::fstream(argv[2], std::ios::out | std::ios::in |
                                         std::ios::trunc | std::ios::binary);
    Execute0();
    return 0;
}
