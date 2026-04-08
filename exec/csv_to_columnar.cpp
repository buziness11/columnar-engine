#include <exception>
#include "src/csv-rw.h"
#include "src/my-format.h"

#if defined(__APPLE__)
#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED  // macos only
#endif
#include <boost/stacktrace.hpp>
#include <csignal>

std::string hits_small_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_sample.csv");
std::string hits_scheme_csv_way_lf =
    TEST_DATA_DIR + std::string("hits_scheme.csv");

void my_handler(int signum) {  // gemini handler
    // Безопасный сброс стека в файл (даже если память повреждена)
    boost::stacktrace::safe_dump_to("crash_stack.dmp");

    // Или быстрый вывод в stderr (но это менее безопасно при повреждении кучи)
    DLOG(INFO) << boost::stacktrace::stacktrace();

    std::exit(signum);
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_alsologtostderr = true;
    for (int sig : {SIGSEGV, SIGABRT, SIGFPE, SIGILL}) {
        std::signal(sig, my_handler);
    }
    if (argc != 4) {
        DLOG(ERROR) << "wrong csv to columnar args count";
        throw std::exception();
    }

    std::fstream table(argv[1], std::ios::in | std::ios::binary);
    if (!table.is_open()) {
        DLOG(ERROR) << "cannot open csv file: " << argv[2];
        throw std::exception();
    }
    std::fstream schema_file(argv[2], std::ios::in | std::ios::binary);
    if (!schema_file.is_open()) {
        DLOG(ERROR) << "cannot open schema file: " << argv[1];
        throw std::exception();
    }
    std::fstream maformat_file(argv[3], std::ios::out | std::ios::in |
                                            std::ios::trunc | std::ios::binary);
    if (!maformat_file.is_open()) {
        DLOG(ERROR) << "cannot open out bzn file: " << argv[2];
        throw std::exception();
    }

    Schema schema(&schema_file);
    CSVReader csv_r(&table, schema.GetCntColumns());
    BZNWriter bzn_w(schema, &maformat_file);
    while (!csv_r.IsReaded()) {
        bzn_w.Write(csv_r.GetBatch());
    }
    bzn_w.WriteMetaInfo();
}
