#include <cstddef>
#include <cstdlib>
#include <exception>
#include <functional>
#include <optional>
#include <string>
#include "core/batch.h"
#include "core/column.h"
#include "core/schema.h"
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
    std::vector<AggregateType> aggs = {AggregateType::Sum,
                                       AggregateType::Count,
                                       AggregateType::Avg};
    std::vector<exe::ExprPtr> exprs = {AdvEngineID,
                                       AdvEngineID,
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
        AggregateOperator(scan,
                          {AggregateType::Min, AggregateType::Max},
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
        filter,
        std::vector<AggregateType>{AggregateType::Count},
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
        scan,
        std::vector<AggregateType>{AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{RegionID});

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
        std::vector<AggregateType>{AggregateType::Sum,
                                   AggregateType::Count,
                                   AggregateType::Avg,
                                   AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{AdvEngineID,
                                  RegionID,
                                  ResolutionWidth,
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
                                                   CmpType::Neq,
                                                   EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::CountDistinct},
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
                                                   CmpType::Neq,
                                                   EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::CountDistinct},
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
        filter,
        std::vector<AggregateType>{AggregateType::Count},
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
        filter,
        std::vector<AggregateType>{AggregateType::CountDistinct},
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
        filter,
        std::vector<AggregateType>{AggregateType::Count},
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
        scan,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{UserID});

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
        scan,
        std::vector<AggregateType>{AggregateType::Count},
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
        scan,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{UserID, SearchPhrase});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_UserID");
    exe::OperPtr limit = std::make_shared<LimitOperator>(group_by, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*limit->Next());
}

// SELECT UserID, extract(minute FROM EventTime) AS m, SearchPhrase, COUNT(*)
// FROM hits GROUP BY UserID, m, SearchPhrase ORDER BY COUNT(*) DESC LIMIT 10;
void Execute18() {
    DLOG(INFO) << "18th que";
    exe::OperPtr scan = GetScanner({"UserID", "EventTime", "SearchPhrase"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr EventTime = std::make_shared<ColumnRef>("EventTime");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");

    exe::ExprPtr m = std::make_shared<ExtractFromTime>(EventTime, "m");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{UserID},
        std::vector<exe::ExprPtr>{UserID, m, SearchPhrase});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_UserID");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute19() {
    DLOG(INFO) << "19th que";
    exe::OperPtr scan = GetScanner({"UserID"});
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");
    exe::ExprPtr UserIDLiteral =
        std::make_shared<Literal<int64_t>>(435090932899640449LL,
                                           Types::kInt64_t);
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

// SELECT SearchPhrase, MIN(URL), COUNT(*) AS c FROM hits WHERE URL LIKE
// '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT
// 10;
void Execute21() {
    DLOG(INFO) << "21st que";
    exe::OperPtr scan = GetScanner({"SearchPhrase", "URL"});
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");

    exe::ExprPtr LikeExpr = std::make_shared<Like>(URL, "google");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr NeqEmpty =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::ExprPtr And =
        std::make_shared<BinaryFunc>(LikeExpr, FuncType::And, NeqEmpty);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        GroupByOperator(filter,
                        {AggregateType::Min, AggregateType::Count},
                        {
                            URL,
                            SearchPhrase,
                        },
                        {SearchPhrase}));

    exe::ExprPtr count_SearchPhrase =
        std::make_shared<ColumnRef>("count_SearchPhrase");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, SearchPhrase, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

// SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(*) AS c, COUNT(DISTINCT
// UserID) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%'
// AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;
void Execute22() {
    DLOG(INFO) << "22nd que";
    exe::OperPtr scan = GetScanner({"SearchPhrase", "URL", "Title", "UserID"});
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr Title = std::make_shared<ColumnRef>("Title");
    exe::ExprPtr UserID = std::make_shared<ColumnRef>("UserID");

    exe::ExprPtr TitleLike = std::make_shared<Like>(Title, "Google");
    exe::ExprPtr UrlNotLike = std::make_shared<Like>(URL, ".google.", true);
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr SearchPhraseNotEmpty =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);

    exe::ExprPtr TitleAndUrl =
        std::make_shared<BinaryFunc>(TitleLike, FuncType::And, UrlNotLike);
    exe::ExprPtr Predicate = std::make_shared<BinaryFunc>(TitleAndUrl,
                                                          FuncType::And,
                                                          SearchPhraseNotEmpty);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Predicate);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Min,
                                   AggregateType::Min,
                                   AggregateType::Count,
                                   AggregateType::CountDistinct},
        std::vector<exe::ExprPtr>{URL, Title, SearchPhrase, UserID},
        std::vector<exe::ExprPtr>{SearchPhrase});

    exe::ExprPtr count_SearchPhrase =
        std::make_shared<ColumnRef>("count_SearchPhrase");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by,
                                               count_SearchPhrase,
                                               10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
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

// SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime
// LIMIT 10;
void Execute24() {
    DLOG(INFO) << "24th que";
    exe::OperPtr scan = GetScanner({"EventTime", "SearchPhrase"});
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

// add non desc order, add out operator
void Execute25() {
    DLOG(INFO) << "25th que";
    exe::OperPtr scan = GetScanner({"SearchPhrase"});
    exe::ExprPtr SearchPhrase = std::make_shared<ColumnRef>("SearchPhrase");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Cmp =
        std::make_shared<BinaryCmp>(SearchPhrase, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, Cmp);
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(filter, SearchPhrase, 10, false);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute26() {
    DLOG(INFO) << "26th que is not supported: ORDER BY multiple columns";
}
void Execute27() {
    DLOG(INFO) << "27th que";
    exe::OperPtr scan = GetScanner({"CounterID", "URL"});
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr UrlNotEmpty =
        std::make_shared<BinaryCmp>(URL, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, UrlNotEmpty);

    exe::ExprPtr UrlLength = std::make_shared<Length>(URL, "length_URL");
    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Avg, AggregateType::Count},
        std::vector<exe::ExprPtr>{UrlLength, URL},
        std::vector<exe::ExprPtr>{CounterID});

    exe::ExprPtr CountUrl = std::make_shared<ColumnRef>("count_URL");
    exe::ExprPtr HavingLiteral =
        std::make_shared<Literal<int64_t>>(100000, Types::kInt64_t);
    exe::ExprPtr Having =
        std::make_shared<BinaryCmp>(CountUrl, CmpType::G, HavingLiteral);
    exe::OperPtr having_filter =
        std::make_shared<FilterOperator>(group_by, Having);

    exe::ExprPtr AvgLength = std::make_shared<ColumnRef>("avg_length_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(having_filter, AvgLength, 25);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute28() {
    DLOG(INFO) << "28th que";
    exe::OperPtr scan = GetScanner({"Referer"});
    exe::ExprPtr Referer = std::make_shared<ColumnRef>("Referer");
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr RefererNotEmpty =
        std::make_shared<BinaryCmp>(Referer, CmpType::Neq, EmptyLiteral);
    exe::OperPtr filter =
        std::make_shared<FilterOperator>(scan, RefererNotEmpty);

    exe::ExprPtr Host =
        std::make_shared<RegexpReplace>(Referer,
                                        "^https?://(?:www\\.)?([^/]+)/.*$",
                                        "\\1",
                                        "k");
    exe::ExprPtr RefererLength =
        std::make_shared<Length>(Referer, "length_Referer");
    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Avg,
                                   AggregateType::Count,
                                   AggregateType::Min},
        std::vector<exe::ExprPtr>{RefererLength, Referer, Referer},
        std::vector<exe::ExprPtr>{Host});

    exe::ExprPtr CountReferer = std::make_shared<ColumnRef>("count_Referer");
    exe::ExprPtr HavingLiteral =
        std::make_shared<Literal<int64_t>>(100000, Types::kInt64_t);
    exe::ExprPtr Having =
        std::make_shared<BinaryCmp>(CountReferer, CmpType::G, HavingLiteral);
    exe::OperPtr having_filter =
        std::make_shared<FilterOperator>(group_by, Having);

    exe::ExprPtr AvgLength = std::make_shared<ColumnRef>("avg_length_Referer");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(having_filter, AvgLength, 25);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute29() {
    DLOG(INFO) << "29th que";
    exe::OperPtr scan = GetScanner({"ResolutionWidth"});
    exe::ExprPtr ResolutionWidth =
        std::make_shared<ColumnRef>("ResolutionWidth");

    std::vector<AggregateType> aggs(90, AggregateType::Sum);
    std::vector<exe::ExprPtr> exprs;
    exprs.reserve(90);
    exprs.emplace_back(ResolutionWidth);
    for (int16_t i = 1; i < 90; ++i) {
        exprs.emplace_back(std::make_shared<BinaryFunc>(
            ResolutionWidth,
            FuncType::Plus,
            std::make_shared<Literal<int16_t>>(i,
                                               Types::kInt16_t,
                                               std::to_string(i)),
            "ResolutionWidth_plus_" + std::to_string(i)));
    }

    exe::OperPtr agg_op =
        std::make_shared<AggregateOperator>(scan, aggs, exprs);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*(agg_op->Next()));
}
void Execute30() {
    DLOG(INFO) << "30th que";
    exe::OperPtr scan = GetScanner({"SearchEngineID",
                                    "ClientIP",
                                    "SearchPhrase",
                                    "IsRefresh",
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
        std::vector<AggregateType>{AggregateType::Count,
                                   AggregateType::Sum,
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
    exe::OperPtr scan = GetScanner({"WatchID",
                                    "ClientIP",
                                    "SearchPhrase",
                                    "IsRefresh",
                                    "ResolutionWidth"});
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
        std::vector<AggregateType>{AggregateType::Count,
                                   AggregateType::Sum,
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
        std::vector<AggregateType>{AggregateType::Count,
                                   AggregateType::Sum,
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
        scan,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL},
        std::vector<exe::ExprPtr>{URL});

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
        scan,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL},
        std::vector<exe::ExprPtr>{One, URL});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

// SELECT ClientIP, ClientIP - 1, ClientIP - 2, ClientIP - 3, COUNT(*) AS c FROM
// hits GROUP BY ClientIP, ClientIP - 1, ClientIP - 2, ClientIP - 3 ORDER BY c
// DESC LIMIT 10;
void Execute35() {
    DLOG(INFO) << "35th que";
    exe::OperPtr scan = GetScanner({"ClientIP"});
    exe::ExprPtr ClientIP = std::make_shared<ColumnRef>("ClientIP");

    exe::ExprPtr ClientIP_minus_1 = std::make_shared<BinaryFunc>(
        ClientIP,
        FuncType::Minus,
        std::make_shared<Literal<int32_t>>(1, Types::kInt32_t, "1"),
        "ClientIP_minus_1");
    exe::ExprPtr ClientIP_minus_2 = std::make_shared<BinaryFunc>(
        ClientIP,
        FuncType::Minus,
        std::make_shared<Literal<int32_t>>(2, Types::kInt32_t, "2"),
        "ClientIP_minus_2");
    exe::ExprPtr ClientIP_minus_3 = std::make_shared<BinaryFunc>(
        ClientIP,
        FuncType::Minus,
        std::make_shared<Literal<int32_t>>(3, Types::kInt32_t, "3"),
        "ClientIP_minus_3");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        scan,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{ClientIP},
        std::vector<exe::ExprPtr>{ClientIP,
                                  ClientIP_minus_1,
                                  ClientIP_minus_2,
                                  ClientIP_minus_3});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_ClientIP");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

// SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND
// EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0
// AND IsRefresh = 0 AND URL <> '' GROUP BY URL ORDER BY PageViews DESC LIMIT
// 10;
void Execute36() {
    DLOG(INFO) << "36th que";
    exe::OperPtr scan = GetScanner(
        {"URL", "CounterID", "EventDate", "DontCountHits", "IsRefresh"});
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr DontCountHits = std::make_shared<ColumnRef>("DontCountHits");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));

    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-01"),
                                           Types::kDate),
        "EventDate_Geq");

    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-31"),
                                           Types::kDate));

    exe::ExprPtr DontCountHits_0 = std::make_shared<BinaryCmp>(
        DontCountHits,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr URL_Neq = std::make_shared<BinaryCmp>(
        URL,
        CmpType::Neq,
        std::make_shared<Literal<std::string>>("", Types::kString));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, DontCountHits_0);
    exe::ExprPtr And4 =
        std::make_shared<BinaryFunc>(And3, FuncType::And, IsRefresh_0);
    exe::ExprPtr And5 =
        std::make_shared<BinaryFunc>(And4, FuncType::And, URL_Neq);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And5);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL},
        std::vector<exe::ExprPtr>{URL});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}

// SELECT Title, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND
// EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0
// AND IsRefresh = 0 AND Title <> '' GROUP BY Title ORDER BY PageViews DESC
// LIMIT 10;
void Execute37() {
    DLOG(INFO) << "37th que";
    exe::OperPtr scan = GetScanner(
        {"Title", "CounterID", "EventDate", "DontCountHits", "IsRefresh"});
    exe::ExprPtr Title = std::make_shared<ColumnRef>("Title");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr DontCountHits = std::make_shared<ColumnRef>("DontCountHits");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));

    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-01"),
                                           Types::kDate));

    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-31"),
                                           Types::kDate));

    exe::ExprPtr DontCountHits_0 = std::make_shared<BinaryCmp>(
        DontCountHits,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr Title_Neq = std::make_shared<BinaryCmp>(
        Title,
        CmpType::Neq,
        std::make_shared<Literal<std::string>>("", Types::kString));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, DontCountHits_0);
    exe::ExprPtr And4 =
        std::make_shared<BinaryFunc>(And3, FuncType::And, IsRefresh_0);
    exe::ExprPtr And5 =
        std::make_shared<BinaryFunc>(And4, FuncType::And, Title_Neq);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And5);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{Title},
        std::vector<exe::ExprPtr>{Title});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_Title");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*order_by_limit->Next());
}
void Execute38() {
    DLOG(INFO) << "38th que";
    exe::OperPtr scan = GetScanner(
        {"URL", "CounterID", "EventDate", "IsRefresh", "IsLink", "IsDownload"});
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr IsLink = std::make_shared<ColumnRef>("IsLink");
    exe::ExprPtr IsDownload = std::make_shared<ColumnRef>("IsDownload");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));

    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-01"),
                                           Types::kDate));

    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-31"),
                                           Types::kDate));

    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr IsLink_Neq_0 = std::make_shared<BinaryCmp>(
        IsLink,
        CmpType::Neq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr IsDownload_0 = std::make_shared<BinaryCmp>(
        IsDownload,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, IsRefresh_0);
    exe::ExprPtr And4 =
        std::make_shared<BinaryFunc>(And3, FuncType::And, IsLink_Neq_0);
    exe::ExprPtr And5 =
        std::make_shared<BinaryFunc>(And4, FuncType::And, IsDownload_0);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And5);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL},
        std::vector<exe::ExprPtr>{URL});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10 + 1000);

    exe::OperPtr offset_op =
        std::make_shared<OffsetOperator>(order_by_limit, 1000);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*offset_op->Next());
}
void Execute39() {
    DLOG(INFO) << "39th que";
    exe::OperPtr scan = GetScanner({"TraficSourceID",
                                    "SearchEngineID",
                                    "AdvEngineID",
                                    "Referer",
                                    "URL",
                                    "CounterID",
                                    "EventDate",
                                    "IsRefresh"});
    exe::ExprPtr TraficSourceID = std::make_shared<ColumnRef>("TraficSourceID");
    exe::ExprPtr SearchEngineID = std::make_shared<ColumnRef>("SearchEngineID");
    exe::ExprPtr AdvEngineID = std::make_shared<ColumnRef>("AdvEngineID");
    exe::ExprPtr Referer = std::make_shared<ColumnRef>("Referer");
    exe::ExprPtr URL = std::make_shared<ColumnRef>("URL");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));
    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-01"),
                                           Types::kDate));
    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-31"),
                                           Types::kDate));
    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, IsRefresh_0);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And3);

    exe::ExprPtr SearchEngineID_0 = std::make_shared<BinaryCmp>(
        SearchEngineID,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));
    exe::ExprPtr AdvEngineID_0 = std::make_shared<BinaryCmp>(
        AdvEngineID,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));
    exe::ExprPtr CasePredicate = std::make_shared<BinaryFunc>(SearchEngineID_0,
                                                              FuncType::And,
                                                              AdvEngineID_0);
    exe::ExprPtr EmptyLiteral =
        std::make_shared<Literal<std::string>>("", Types::kString);
    exe::ExprPtr Src =
        std::make_shared<Case>(CasePredicate, Referer, EmptyLiteral, "Src");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URL},
        std::vector<exe::ExprPtr>{TraficSourceID,
                                  SearchEngineID,
                                  AdvEngineID,
                                  Src,
                                  URL});

    exe::ExprPtr PageViews = std::make_shared<ColumnRef>("count_URL");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, PageViews, 10 + 1000);
    exe::OperPtr offset_op =
        std::make_shared<OffsetOperator>(order_by_limit, 1000);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*offset_op->Next());
}
void Execute40() {
    DLOG(INFO) << "40th que";
    exe::OperPtr scan = GetScanner({"URLHash",
                                    "EventDate",
                                    "CounterID",
                                    "IsRefresh",
                                    "TraficSourceID",
                                    "RefererHash"});
    exe::ExprPtr URLHash = std::make_shared<ColumnRef>("URLHash");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr TraficSourceID = std::make_shared<ColumnRef>("TraficSourceID");
    exe::ExprPtr RefererHash = std::make_shared<ColumnRef>("RefererHash");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));

    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-01"),
                                           Types::kDate));

    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-31"),
                                           Types::kDate));

    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr TraficSourceID_minus1 = std::make_shared<BinaryCmp>(
        TraficSourceID,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(-1, Types::kInt16_t));
    exe::ExprPtr TraficSourceID_6 = std::make_shared<BinaryCmp>(
        TraficSourceID,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(6, Types::kInt16_t));
    exe::ExprPtr TraficSourceID_In =
        std::make_shared<BinaryFunc>(TraficSourceID_minus1,
                                     FuncType::Or,
                                     TraficSourceID_6);

    exe::ExprPtr RefererHash_Eq = std::make_shared<BinaryCmp>(
        RefererHash,
        CmpType::Eq,
        std::make_shared<Literal<int64_t>>(3594120000172545465LL,
                                           Types::kInt64_t));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, IsRefresh_0);
    exe::ExprPtr And4 =
        std::make_shared<BinaryFunc>(And3, FuncType::And, TraficSourceID_In);
    exe::ExprPtr And5 =
        std::make_shared<BinaryFunc>(And4, FuncType::And, RefererHash_Eq);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And5);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{URLHash},
        std::vector<exe::ExprPtr>{URLHash, EventDate});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_URLHash");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10 + 100);

    exe::OperPtr offset_op =
        std::make_shared<OffsetOperator>(order_by_limit, 100);

    CsvWriter csv_w(&res_file);
    csv_w.WriteBatch(*offset_op->Next());
}
void Execute41() {
    DLOG(INFO) << "41st que";
    exe::OperPtr scan = GetScanner({"WindowClientWidth",
                                    "WindowClientHeight",
                                    "CounterID",
                                    "EventDate",
                                    "IsRefresh",
                                    "DontCountHits",
                                    "URLHash"});
    exe::ExprPtr WindowClientWidth =
        std::make_shared<ColumnRef>("WindowClientWidth");
    exe::ExprPtr WindowClientHeight =
        std::make_shared<ColumnRef>("WindowClientHeight");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr DontCountHits = std::make_shared<ColumnRef>("DontCountHits");
    exe::ExprPtr URLHash = std::make_shared<ColumnRef>("URLHash");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));

    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-01"),
                                           Types::kDate));

    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-31"),
                                           Types::kDate));

    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr DontCountHits_0 = std::make_shared<BinaryCmp>(
        DontCountHits,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr URLHash_Eq = std::make_shared<BinaryCmp>(
        URLHash,
        CmpType::Eq,
        std::make_shared<Literal<int64_t>>(2868770270353813622LL,
                                           Types::kInt64_t));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, IsRefresh_0);
    exe::ExprPtr And4 =
        std::make_shared<BinaryFunc>(And3, FuncType::And, DontCountHits_0);
    exe::ExprPtr And5 =
        std::make_shared<BinaryFunc>(And4, FuncType::And, URLHash_Eq);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And5);

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{WindowClientWidth},
        std::vector<exe::ExprPtr>{WindowClientWidth, WindowClientHeight});

    exe::ExprPtr c = std::make_shared<ColumnRef>("count_WindowClientWidth");
    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by, c, 10 + 10000);

    exe::OperPtr offset_op =
        std::make_shared<OffsetOperator>(order_by_limit, 10000);

    CsvWriter csv_w(&res_file);
    std::optional<Batch> res = offset_op->Next();
    if (!res.has_value()) {
        return;
    }
    csv_w.WriteBatch(*res);
}
void Execute42() {
    DLOG(INFO) << "42nd que";
    exe::OperPtr scan = GetScanner(
        {"EventTime", "CounterID", "EventDate", "IsRefresh", "DontCountHits"});
    exe::ExprPtr EventTime = std::make_shared<ColumnRef>("EventTime");
    exe::ExprPtr CounterID = std::make_shared<ColumnRef>("CounterID");
    exe::ExprPtr EventDate = std::make_shared<ColumnRef>("EventDate");
    exe::ExprPtr IsRefresh = std::make_shared<ColumnRef>("IsRefresh");
    exe::ExprPtr DontCountHits = std::make_shared<ColumnRef>("DontCountHits");

    exe::ExprPtr CounterID_62 = std::make_shared<BinaryCmp>(
        CounterID,
        CmpType::Eq,
        std::make_shared<Literal<int32_t>>(62, Types::kInt32_t));

    exe::ExprPtr EventDate_Geq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Geq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-14"),
                                           Types::kDate));

    exe::ExprPtr EventDate_Leq = std::make_shared<BinaryCmp>(
        EventDate,
        CmpType::Leq,
        std::make_shared<Literal<int32_t>>(DaysCount("2013-07-15"),
                                           Types::kDate));

    exe::ExprPtr IsRefresh_0 = std::make_shared<BinaryCmp>(
        IsRefresh,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr DontCountHits_0 = std::make_shared<BinaryCmp>(
        DontCountHits,
        CmpType::Eq,
        std::make_shared<Literal<int16_t>>(0, Types::kInt16_t));

    exe::ExprPtr And1 = std::make_shared<BinaryFunc>(CounterID_62,
                                                     FuncType::And,
                                                     EventDate_Geq);
    exe::ExprPtr And2 =
        std::make_shared<BinaryFunc>(And1, FuncType::And, EventDate_Leq);
    exe::ExprPtr And3 =
        std::make_shared<BinaryFunc>(And2, FuncType::And, IsRefresh_0);
    exe::ExprPtr And4 =
        std::make_shared<BinaryFunc>(And3, FuncType::And, DontCountHits_0);

    exe::OperPtr filter = std::make_shared<FilterOperator>(scan, And4);

    exe::ExprPtr TruncEventTime =
        std::make_shared<TruncateTime>(EventTime, Trunc::KMinutes, "M");

    exe::OperPtr group_by = std::make_shared<GroupByOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<exe::ExprPtr>{TruncEventTime},
        std::vector<exe::ExprPtr>{TruncEventTime});

    exe::ExprPtr TruncEvent = std::make_shared<ColumnRef>("M");

    exe::OperPtr order_by_limit =
        std::make_shared<OrderByLimitOperator>(group_by,
                                               TruncEvent,
                                               10 + 1000,
                                               false);

    exe::OperPtr offset_op =
        std::make_shared<OffsetOperator>(order_by_limit, 1000);

    CsvWriter csv_w(&res_file);
    std::optional<Batch> res = offset_op->Next();
    if (!res.has_value()) {
        return;
    }
    csv_w.WriteBatch(*res);
}

void my_handler() {  // gemini handler
    std::signal(SIGABRT, SIG_DFL);

    auto eptr = std::current_exception();
    if (eptr) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            LOG(ERROR) << "Uncaught exception: " << e.what();
        } catch (...) {
            LOG(ERROR) << "Unknown exception";
        }
    }

    LOG(ERROR) << boost::stacktrace::stacktrace();
    google::FlushLogFiles(google::GLOG_ERROR);  // важно!

    std::abort();  // теперь без рекурсии
}

void my_handler(int signum) {  // gemini handler
    DLOG(INFO) << boost::stacktrace::stacktrace();
    google::FlushLogFiles(google::INFO);  // флаш glog
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
    std::set_terminate(my_handler);
    for (int sig : {SIGSEGV, SIGABRT, SIGFPE, SIGILL}) {
        std::signal(sig, my_handler);
    }
    maformat_file =
        std::fstream(argv[2], std::ios::out | std::ios::in | std::ios::binary);
    res_file = std::fstream(
        argv[3],
        std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
    size_t idx = std::stoi(argv[1]);
    if (idx < executors.size()) {
        executors[idx]();
    } else {
        DLOG(INFO) << "dont support " << idx << " question now can only"
                   << executors.size();
    }
    return 0;
}
