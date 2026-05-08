#include <cstddef>
#include <functional>
#include <string>
#include "core/batch.h"
#include "io/csv-rw.h"
#include "io/my-format.h"
#include "query/operators.h"
#include "query/expressions.h"
#include "core/types.h"
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

    CsvWriter csv_w(&res_file);
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

    CsvWriter csv_w(&res_file);
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

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*(agg_op->Next()));
}

void Execute3() {
    DLOG(INFO) << "3rd que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>(ColumnRef("UserID"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::Avg}, {UserID}).Next();

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute4() {
    DLOG(INFO) << "4th que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>(ColumnRef("UserID"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::CountDistinct}, {UserID})
            .Next();

    CsvWriter csv_w(&res_file);
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

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute6() {
    DLOG(INFO) << "6th que";
    exe::OperPtr scan = GetScanner({"EventDate"});
    exe::ExprPtr EventDate =
        std::make_shared<ColumnRef>(ColumnRef("EventDate"));
    std::optional<Batch> res =
        AggregateOperator(scan, {AggregateType::Min, AggregateType::Max},
                          {EventDate, EventDate})
            .Next();

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*res);
}
void Execute7() {
    DLOG(INFO) << "7th que";
    exe::OperPtr scan = GetScanner({"AdvEngineID"});
    exe::ExprPtr AdvEngineID = std::make_shared<ColumnRef>("AdvEngineID");
    exe::ExprPtr ZeroLiteral =
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(AdvEngineID, CmpType::Neq, ZeroLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{AdvEngineID},
        std::vector<exe::ExprPtr>{AdvEngineID});

    exe::ExprPtr count_AdvEngineID =
        std::make_shared<ColumnRef>(ColumnRef("count_AdvEngineID"));

    exe::OperPtr order_by =
        std::make_shared<OrderByOperator>(group_by, count_AdvEngineID);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by->Next());
}

void Execute8() {
    DLOG(INFO) << "8th que";
    exe::OperPtr scan = GetScanner({"RegionID", "UserID"});
    exe::ExprPtr RegionID = std::make_shared<ColumnRef>("RegionID");
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan, std::vector<AggregateType>{AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{UserID}, std::vector<exe::ExprPtr>{RegionID});

    exe::ExprPtr u = std::make_shared<ColumnRef>("count_distinct_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, u, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute9() {
    DLOG(INFO) << "9th que";
    exe::OperPtr scan =
        GetScanner({"RegionID", "AdvEngineID", "ResolutionWidth", "UserID"});
    exe::ExprPtr RegionID = std::make_shared<ColumnRef>("RegionID");
    exe::ExprPtr AdvEngineID = std::make_shared<ColumnRef>("AdvEngineID");
    exe::ExprPtr ResolutionWidth =
        std::make_shared<ColumnRef>("ResolutionWidth");
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan,
        std::vector<AggregateType>{AggregateType::Sum, AggregateType::Count,
                                   AggregateType::Avg,
                                   AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{AdvEngineID, RegionID, ResolutionWidth,
                                  UserID},
        std::vector<exe::ExprPtr>{RegionID});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_RegionID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

void Execute10() {
    DLOG(INFO) << "10th que";
    exe::OperPtr scan = GetScanner({"MobilePhoneModel", "UserID"});
    exe::ExprPtr MobilePhoneModel =
        std::make_shared<ColumnRef>("MobilePhoneModel");
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp = std::make_shared<BinaryCmp>(MobilePhoneModel,
                                                   CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter, std::vector<AggregateType>{AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{MobilePhoneModel});

    exe::ExprPtr u = std::make_shared<ColumnRef>("count_distinct_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, u, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute11() {
    DLOG(INFO) << "11th que";
    exe::OperPtr scan =
        GetScanner({"MobilePhone", "MobilePhoneModel", "UserID"});
    exe::ExprPtr MobilePhone = std::make_shared<ColumnRef>("MobilePhone");
    exe::ExprPtr MobilePhoneModel =
        std::make_shared<ColumnRef>("MobilePhoneModel");
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp = std::make_shared<BinaryCmp>(MobilePhoneModel,
                                                   CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter, std::vector<AggregateType>{AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{MobilePhone, MobilePhoneModel});

    exe::ExprPtr u = std::make_shared<ColumnRef>("count_distinct_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, u, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

void Execute12() {
    DLOG(INFO) << "12th que";
    exe::OperPtr scan = GetScanner({"SearchPhrase"});
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{SearchPhrase},
        std::vector<exe::ExprPtr>{SearchPhrase});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_SearchPhrase");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute13() {
    DLOG(INFO) << "13th que";
    exe::OperPtr scan = GetScanner({"SearchPhrase", "UserID"});
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter, std::vector<AggregateType>{AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{SearchPhrase});

    exe::ExprPtr u = std::make_shared<ColumnRef>("count_distinct_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, u, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

void Execute14() {
    DLOG(INFO) << "14th que";
    exe::OperPtr scan = GetScanner({"SearchEngineID", "SearchPhrase"});
    exe::ExprPtr SearchEngineID = std::make_shared<ColumnRef>("SearchEngineID");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{SearchPhrase},
        std::vector<exe::ExprPtr>{SearchEngineID, SearchPhrase});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_SearchPhrase");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute15() {
    DLOG(INFO) << "15th que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{UserID}, std::vector<exe::ExprPtr>{UserID});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

void Execute16() {
    DLOG(INFO) << "16th que";
    exe::OperPtr scan = GetScanner({"UserID", "SearchPhrase"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{UserID, SearchPhrase});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute17() {
    DLOG(INFO) << "17th que";
    exe::OperPtr scan = GetScanner({"UserID", "SearchPhrase"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{UserID, SearchPhrase});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_UserID");
    exe::OperPtr limit = std::make_shared<LimitOperator>(group_by, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*limit->Next());
}

void Execute18() {
    DLOG(INFO) << "18th que is not supported: extract(minute FROM EventTime)";
}
void Execute19() {
    DLOG(INFO) << "19th que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr UserIDLiteral = std::make_shared<Literal<int64_t>>(
        435090932899640449LL, Types::kInt64_t);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(UserID, CmpType::Eq, UserIDLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*filter->Next());
}

// SELECT COUNT(*) FROM hits WHERE URL LIKE '%google%';
void Execute20() {
    DLOG(INFO) << "20th que";
    exe::OperPtr scan = GetScanner({"URL"});
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr LikeExpr = std::make_shared<Like>(URL, "google");
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, LikeExpr);
    exe::OperPtr count = std::make_shared<AggregateOperator>(
        AggregateOperator(filter, {AggregateType::Count}, {URL}));

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*count->Next());
}
void Execute21() {
    DLOG(INFO) << "21st que is not supported: AND and MIN(string)";
}

void Execute22() {
    DLOG(INFO) << "22nd que is not supported: AND, NOT LIKE and MIN(string)";
}
void Execute23() {
    DLOG(INFO) << "23rd que";
    exe::OperPtr scan = GetScanner({});
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr LikeExpr = std::make_shared<Like>(URL, "google");
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, LikeExpr);
    exe::ExprPtr EventTime = std::make_shared<ColumnRef>("EventTime");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(filter, EventTime, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

void Execute24() {
    DLOG(INFO) << "24th que is not supported: projection after ORDER BY";
}

// add non desc order, add out operator
void Execute25() {
    DLOG(INFO) << "25th que";
    exe::OperPtr scan = GetScanner({"SearchPhrase", "EventTime"});
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr EventTime = std::make_shared<ColumnRef>("EventTime");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(filter, EventTime, 10, false);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute26() {
    DLOG(INFO) << "26th que is not supported: ORDER BY multiple columns";
}
void Execute27() {
    DLOG(INFO) << "27th que is not supported: length() and HAVING";
}
void Execute28() {
    DLOG(INFO)
        << "28th que is not supported: REGEXP_REPLACE, length() and HAVING";
}
void Execute29() {
    DLOG(INFO) << "29th que is not supported: arithmetic expressions";
}
void Execute30() {
    DLOG(INFO) << "30th que";
    exe::OperPtr scan =
        GetScanner({"SearchEngineID", "ClientIP", "SearchPhrase", "IsRefresh",
                    "ResolutionWidth"});
    exe::ExprPtr SearchEngineID = std::make_shared<ColumnRef>("SearchEngineID");
    exe::ExprPtr ClientIP = std::make_shared<ColumnRef>("ClientIP");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr ResolutionWidth =
        std::make_shared<ColumnRef>("ResolutionWidth");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count, AggregateType::Sum,
                                   AggregateType::Avg},
        std::vector<exe::ExprPtr>{SearchPhrase, IsRefresh, ResolutionWidth},
        std::vector<exe::ExprPtr>{SearchEngineID, ClientIP});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_SearchPhrase");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute31() {
    DLOG(INFO) << "31st que";
    exe::OperPtr scan = GetScanner({"WatchID", "ClientIP", "SearchPhrase",
                                    "IsRefresh", "ResolutionWidth"});
    exe::ExprPtr WatchID = std::make_shared<ColumnRef>("WatchID");
    exe::ExprPtr ClientIP = std::make_shared<ColumnRef>("ClientIP");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr ResolutionWidth =
        std::make_shared<ColumnRef>("ResolutionWidth");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count, AggregateType::Sum,
                                   AggregateType::Avg},
        std::vector<exe::ExprPtr>{SearchPhrase, IsRefresh, ResolutionWidth},
        std::vector<exe::ExprPtr>{WatchID, ClientIP});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_SearchPhrase");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute32() {
    DLOG(INFO) << "32nd que";
    exe::OperPtr scan =
        GetScanner({"WatchID", "ClientIP", "IsRefresh", "ResolutionWidth"});
    exe::ExprPtr WatchID = std::make_shared<ColumnRef>("WatchID");
    exe::ExprPtr ClientIP = std::make_shared<ColumnRef>("ClientIP");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr ResolutionWidth =
        std::make_shared<ColumnRef>("ResolutionWidth");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan,
        std::vector<AggregateType>{AggregateType::Count, AggregateType::Sum,
                                   AggregateType::Avg},
        std::vector<exe::ExprPtr>{WatchID, IsRefresh, ResolutionWidth},
        std::vector<exe::ExprPtr>{WatchID, ClientIP});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_WatchID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute33() {
    DLOG(INFO) << "33rd que";
    exe::OperPtr scan = GetScanner({"URL"});
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL}, std::vector<exe::ExprPtr>{URL});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute34() {
    DLOG(INFO) << "34th que";
    exe::OperPtr scan = GetScanner({"URL"});
    exe::ExprPtr One =
        std::make_shared<Literal<int16_t>>(1, Types::kInt16_t, "1");
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan, std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL}, std::vector<exe::ExprPtr>{One, URL});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute35() {
    DLOG(INFO) << "35th que is not supported: arithmetic expressions in "
                  "SELECT/GROUP BY";
}
void Execute36() {
    DLOG(INFO) << "36th que is not supported: AND filters";
}
void Execute37() {
    DLOG(INFO) << "37th que is not supported: AND filters";
}
void Execute38() {
    DLOG(INFO) << "38th que is not supported: AND filters and OFFSET";
}
void Execute39() {
    DLOG(INFO) << "39th que is not supported: CASE, AND filters and OFFSET";
}
void Execute40() {
    DLOG(INFO) << "40th que is not supported: IN, AND filters and OFFSET";
}
void Execute41() {
    DLOG(INFO) << "41st que is not supported: AND filters and OFFSET";
}
void Execute42() {
    DLOG(INFO)
        << "42nd que is not supported: DATE_TRUNC, AND filters and OFFSET";
}

void my_handler(int signum) {  // gemini handler
    DLOG(INFO) << boost::stacktrace::stacktrace();
    std::exit(signum);
}

std::vector<std::function<void()>> executors = {
    Execute0,  Execute1,  Execute2,  Execute3,  Execute4,  Execute5,  Execute6,
    Execute7,  Execute8,  Execute9,  Execute10, Execute11, Execute12, Execute13,
    Execute14, Execute15, Execute16, Execute17, Execute18, Execute19, Execute20,
    Execute21, Execute22, Execute23, Execute24, Execute25, Execute26, Execute27,
    Execute28, Execute29, Execute30, Execute31, Execute32, Execute33, Execute34,
    Execute35, Execute36, Execute37, Execute38, Execute39, Execute40, Execute41,
    Execute42};

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
