# Columnar Engine

Учебный колоночный движок аналитической обработки данных на C++20: собственный бинарный формат хранения **BZN**, конвертация из/в CSV и pull‑based исполнитель SQL‑подобных запросов (Volcano‑model) с операторами Scan / Filter / Aggregate / GroupBy / OrderByLimit.

---

## Содержание

- [Возможности](#возможности)
- [Структура проекта](#структура-проекта)
- [Архитектура исходного кода](#архитектура-исходного-кода)
- [Сборка](#сборка)
- [CLI‑утилиты](#cli-утилиты)
- [Типы данных](#типы-данных)
- [Формат BZN](#формат-bzn)
- [Поток выполнения запроса](#поток-выполнения-запроса)
- [Пример программного использования](#пример-программного-использования)
- [Тесты и бенчмарки](#тесты-и-бенчмарки)
- [Зависимости](#зависимости)

---

## Возможности

- Колоночное хранение в собственном формате [`.bzn`](#формат-bzn).
- Конвертация **CSV ↔ BZN** (RFC 4180, поддержка `LF`/`CRLF`).
- Векторизованное чтение/запись батчами по 500 000 строк (`kBatchRowSize`).
- Селективное чтение нужных колонок из BZN без декодирования остальных.
- Volcano‑исполнитель с операторами:
  - [`ScanOperator`](query/operators.h:17)
  - [`FilterOperator`](query/operators.h:28)
  - [`AggregateOperator`](query/operators.h:42)
  - [`GroupByOperator`](query/operators.h:55)
  - [`OrderByLimitOperator`](query/operators.h:70)
- Агрегатные функции: `Count`, `Sum`, `Avg`, `CountDistinct`, `Min`, `Max` (см. [`query/aggregate.h`](query/aggregate.h:1)).
- Поддерживаемые типы: целые `int16/32/64`, `double`, `long double`, `bool`, `string`, `DATE`, `TIMESTAMP`.

---

## Структура проекта

```
columnar-engine/
├── CMakeLists.txt           # Корневой CMake (определяет engine_lib)
├── cmake/                   # Поиск зависимостей (glog, boost)
├── core/                    # Ядро: типы, схема, колонки, батчи
├── io/                      # I/O: CSV, бинарный формат BZN
├── query/                   # SQL-исполнитель: выражения, операторы, агрегаты
├── exec/                    # CLI‑утилиты (csv->bzn, bzn->csv, exec_queries...)
├── tests/                   # Тесты и бенчмарки на CSV/BZN
├── script/                  # build.sh / convert.sh / run_query.sh / setup.sh
├── documents/               # Курсовая работа (LaTeX + PDF)
└── queries.sql              # Примеры SQL‑запросов
```

---

## Архитектура исходного кода

| Файл | Назначение |
|---|---|
| [`core/types.h`](core/types.h:1) / [`core/types.cpp`](core/types.cpp:1) | Перечисление [`Types`](core/types.h:15), вариант [`ColumnType`](core/types.h:27), шаблонные конвертеры [`TranslateTtoU`](core/types.h:54), `EnumToCpp`. |
| [`core/datatype.h`](core/datatype.h:1) / [`core/datatype.cpp`](core/datatype.cpp:1) | Преобразование `DATE` ↔ `int32 days` и `TIMESTAMP` ↔ `int64 seconds`. Диапазон 1970–2040. |
| [`core/rwconsts.h`](core/rwconsts.h:1) | Константы формата (разделители BZN `\x1E`, `\x1F`), размер батча, префиксы дней. |
| [`core/schema.h`](core/schema.h:1) / [`core/schema.cpp`](core/schema.cpp:1) | Класс [`Schema`](core/schema.h:7) — пары `(имя, тип)`. Чтение из CSV‑схемы. |
| [`core/column.h`](core/column.h:1) / [`core/column.cpp`](core/column.cpp:1) | [`Column`](core/column.h:57) поверх `std::variant<std::vector<T>...>`. Сериализация: [`WriteColToBzn`](core/column.cpp:119), [`ReadColFromBzn`](core/column.cpp:146). Шаблонная диспетчеризация типов: [`DispatchColumnHelper`](core/column.h:17). |
| [`core/batch.h`](core/batch.h:1) / [`core/batch.cpp`](core/batch.cpp:1) | [`Batch`](core/batch.h:7) — `Schema + std::vector<Column>`. Доступ по имени/индексу, объединение, выборка строки. |
| [`io/csv-rw.h`](io/csv-rw.h:1) / [`io/csv-rw.cpp`](io/csv-rw.cpp:1) | [`CSVReader`](io/csv-rw.h:12) (RFC 4180, `LF`/`CRLF`) и [`CSVWriter`](io/csv-rw.h:27). |
| [`io/my-format.h`](io/my-format.h:1) / [`io/my-format.cpp`](io/my-format.cpp:1) | Свой бинарный формат BZN: [`BZNReader`](io/my-format.h:11) / [`BZNWriter`](io/my-format.h:29). |
| [`query/expressions.h`](query/expressions.h:1) / [`query/expressions.cpp`](query/expressions.cpp:1) | Дерево выражений: [`IExpression`](query/expressions.h:7), [`ColumnRef`](query/expressions.h:13), [`Literal`](query/expressions.h:24), [`BinaryCmp`](query/expressions.h:40). |
| [`query/operators.h`](query/operators.h:1) / [`query/operators.cpp`](query/operators.cpp:1) | Volcano‑операторы: см. список выше. |
| [`query/aggregate.h`](query/aggregate.h:1) / [`query/aggregate.cpp`](query/aggregate.cpp:1) | Агрегатные функции и состояния. |
| [`CMakeLists.txt`](CMakeLists.txt:1) | Сборка `engine_lib` (cписок исходников через [`core/CMakeLists.txt`](core/CMakeLists.txt:1), [`io/CMakeLists.txt`](io/CMakeLists.txt:1), [`query/CMakeLists.txt`](query/CMakeLists.txt:1)). |

Подробное описание API каждого модуля — в разделе [API модулей](#api-модулей).

Зависимости между модулями:

```
types ─► datatype ─► column ─► batch ─► csv-rw
                                  ├──► my-format
                                  └──► expressions ─► operators ─► aggregate
```

---

## Сборка

Требования:

- CMake ≥ 3.20
- Компилятор с поддержкой C++20 (gcc 11+/clang 14+/AppleClang 15+)
- Boost (header‑only)
- glog
- (опционально) gtest для тестов

Быстрый старт:

```bash
./script/setup.sh        # установка зависимостей через системный пакетный менеджер
./script/build.sh        # cmake -B build && cmake --build build -j
```

или вручную:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Скрипт [`script/build.sh`](script/build.sh) выполняет конфигурацию и сборку проекта.

---

## CLI‑утилиты

Бинари собираются в `build/exec/`.

### csv → bzn

```bash
./build/exec/csv_to_columnar \
    --csv data.csv \
    --schema schema.csv \
    --out data.bzn \
    [--lf|--crlf] [--delim ,]
```

`schema.csv` — двухколоночный CSV `имя,тип` (см. [`StringToType`](core/types.cpp:38) для допустимых имён: `int16`, `int32`, `int64`, `double`, `long double`, `bool`, `string`, `DATE`, `TIMESTAMP`).

### bzn → csv

```bash
./build/exec/columnar_to_csv --in data.bzn --out data.csv
```

### Чтение одной колонки

```bash
./build/exec/get_col_from_bzn --in data.bzn --column UserID
```

### Выполнение запросов

```bash
./build/exec/exec_queries --in data.bzn --queries queries.sql
```

Готовые шорт‑скрипты:

- [`script/convert.sh`](script/convert.sh) — конвертация CSV→BZN.
- [`script/run_query.sh`](script/run_query.sh) — выполнение запроса.

---

## Типы данных

| `Types` | C++ тип | Размер | Описание |
|---|---|---|---|
| `kInt16_t`     | `int16_t`     | 2 | Знаковое целое |
| `kInt32_t`     | `int32_t`     | 4 | Знаковое целое |
| `kInt64_t`     | `int64_t`     | 8 | Знаковое целое |
| `kDouble`      | `double`      | 8 | IEEE 754 |
| `kLongDouble`  | `long double` | 8/16 | platform‑dependent |
| `kBool`        | `bool`        | — | Только in‑memory, не сериализуется в BZN |
| `kString`      | `std::string` | var | UTF‑8, разделитель `\x1F` |
| `kDate`        | `int32_t`     | 4 | Дни с 1970‑01‑01, формат `YYYY-MM-DD` |
| `kTimestamp`   | `int64_t`     | 8 | Секунды с 1970‑01‑01, формат `YYYY-MM-DD HH:MM:SS` |

Преобразования между типами выполняются через [`Column::TranslateTo`](core/column.cpp:55), который инстанцирует подходящую специализацию [`TranslateTtoU`](core/types.h:54).

---

## Формат BZN

Файл состоит из трёх логических частей:

```
+------------------------------+ offset 0
| meta_size: int64 (8 bytes)   |  ← смещение конца данных от конца файла
+------------------------------+ offset 8
| BATCH_0                      |
|   per_column_offsets[ncols]  |  ← начала колонок этого батча, int64 каждый
|   col_0 raw bytes            |
|   col_1 raw bytes            |
|   ...                        |
+------------------------------+
| BATCH_1                      |
|   ...                        |
+------------------------------+
| ...                          |
+------------------------------+ end - meta_size
| names: string \x1F ... \x1E  |
| types: string \x1F ... \x1E  |
| batch_offsets[nbatch+1]: i64 |
+------------------------------+ EOF
```

- Целочисленные/вещественные колонки сериализуются как `value_type` массив подряд (little/native endian, без сжатия).
- Строковые колонки — последовательность байт, разделённых `\x1F` (`kStringDelimiter`).
- `bool` сериализация **не поддерживается** ([`column.cpp:128`](core/column.cpp:128)).
- Метаразделитель блоков мета — `\x1E` (`kMetaDelimiter`).

Чтение реализовано в [`BZNReader`](io/my-format.cpp:19), запись — в [`BZNWriter`](io/my-format.cpp:127). [`BZNReader::Read`](io/my-format.cpp:96) принимает список нужных колонок и пропускает остальные через `seekg`.

---

## Поток выполнения запроса

```
       BZN file
          │
          ▼
   ScanOperator ──► Batch (по 500k строк)
          │
          ▼
   FilterOperator   (применяет IExpression → vector<bool> mask)
          │
          ▼
   GroupByOperator / AggregateOperator
          │           (CountFunc / SumFunc / AvgFunc / MinFunc /
          │            MaxFunc / CountDistinctFunc — IAggregateFunc)
          ▼
   OrderByLimitOperator (top‑K по std::multimap)
          │
          ▼
   результат: Batch
```

Каждый оператор реализует [`IOperator::Next()`](query/operators.h:14) и возвращает следующий батч либо `std::nullopt` при исчерпании.

Выражения формируются вручную через `std::shared_ptr<IExpression>`:

- [`ColumnRef("col")`](query/expressions.h:13) — ссылка на колонку батча по имени.
- [`Literal<T>(val, type)`](query/expressions.h:24) — константа.
- [`BinaryCmp(left, CmpType, right)`](query/expressions.h:40) — сравнение (на момент написания реализован только `Neq`, см. [review.md](review.md)).

---

## Пример программного использования

```cpp
#include "my-format.h"
#include "operators.h"
#include "expressions.h"

int main() {
    // 1. Открыть BZN
    auto file = std::make_unique<std::fstream>("data.bzn",
        std::ios::in | std::ios::binary);

    // 2. Построить план: SELECT Count(*) FROM data WHERE EventDate != 0
    auto scan = std::make_shared<ScanOperator>(
        BZNReader(file.get()),
        std::vector<std::string>{"EventDate"});

    auto pred = std::make_shared<BinaryCmp>(
        std::make_shared<ColumnRef>("EventDate"),
        CmpType::Neq,
        std::make_shared<Literal<int32_t>>(0, Types::kDate));

    auto filter = std::make_shared<FilterOperator>(scan, pred);

    auto agg = std::make_shared<AggregateOperator>(
        filter,
        std::vector<AggregateType>{AggregateType::Count},
        std::vector<std::shared_ptr<IExpression>>{
            std::make_shared<ColumnRef>("EventDate")});

    // 3. Выполнить
    while (auto batch = agg->Next()) {
        batch->GetColumnIdx(0).PrintCol();
    }
}
```

См. также [`exec/exec_queries.cpp`](exec/exec_queries.cpp) — реальное использование операторов в CLI.

---

## Тесты и бенчмарки

```bash
cmake --build build --target test
ctest --test-dir build --output-on-failure
```

- [`tests/test_csv.cpp`](tests/test_csv.cpp) — round‑trip CSV.
- [`tests/test_format.cpp`](tests/test_format.cpp) — round‑trip BZN.
- [`tests/bench_rw_hits.cpp`](tests/bench_rw_hits.cpp) — бенчмарк на датасете HITS.

Тестовые данные лежат в [`tests/data/`](tests/data/).

---

## Зависимости

| Библиотека | Использование |
|---|---|
| **glog** | Логирование (`DLOG(INFO/ERROR/WARNING)`) |
| **Boost** (header‑only) | Утилиты |
| **CMake 3.20+** | Сборка |
| **C++20** | Concepts, `std::variant`, `std::optional`, шаблонные lambdas |

Установка зависимостей описана в [`script/setup.sh`](script/setup.sh).

---

## API модулей

Подробное описание публичных методов каждого файла. Сигнатуры приведены упрощённо; точные ссылки — на исходники.

### `types.h` / `types.cpp`

Базовые перечисления и шаблоны для типизации колонок.

- `enum class Types` — перечень поддерживаемых типов ([`types.h:15`](core/types.h:15)).
- `using ColumnType` — `std::variant` всех `std::vector<T>` ([`types.h:27`](core/types.h:27)).
- `using ValueType` — `std::variant` скаляров ([`types.h:32`](core/types.h:32)).
- `std::string TypeToString(Types)` — текстовое представление ([`types.cpp:4`](core/types.cpp:4)).
- `Types StringToType(const std::string&)` / `Types StringToType(std::string&&)` — обратная конвертация ([`types.cpp:38`](core/types.cpp:38)).
- `template<Types T, Types U> auto TranslateTtoU(X)` — набор шаблонных перегрузок для приведения значения от `Src` к `Dst` (`Numeric ↔ Numeric`, `Numeric ↔ String`, `Date ↔ String`, `Timestamp ↔ String`) ([`types.h:54`](core/types.h:54)).
- `template<Types T> struct EnumToCpp` — отображение `Types` → C++ тип через `EnumToCpp<T>::Type` ([`types.h:101`](core/types.h:101)).
- Концепты `NumericType`, `StringType` — для `requires`‑ограничений шаблонов ([`types.h:44`](core/types.h:44)).

### `datatype.h` / `datatype.cpp`

Преобразование DATE/TIMESTAMP. Поддерживаемый диапазон годов — 1970…2040.

- `int32_t DaysCount(const std::string& yyyy_mm_dd)` — количество дней с 1970‑01‑01 ([`datatype.cpp:14`](core/datatype.cpp:14)).
- `int64_t SecondsCount(const std::string& yyyy_mm_dd_hh_mm_ss)` — секунды с эпохи ([`datatype.cpp:33`](core/datatype.cpp:33)).
- `std::string GetYyyyMmDd(int32_t days)` — обратное преобразование дней в дату ([`datatype.cpp:45`](core/datatype.cpp:45)).
- `std::string GetYyyyMmDdHhMmSs(int64_t seconds)` — секунды в timestamp ([`datatype.cpp:73`](core/datatype.cpp:73)).
- (внутреннее) `inline bool IsLeap(int32_t)` — проверка високосного года.

### `rwconsts.h`

Только константы:

- `kMetaDelimiter = '\x1E'` — конец блока метаинформации.
- `kStringDelimiter = '\x1F'` — разделитель строк в BZN.
- `kBatchRowSize = 500'000` — размер батча по умолчанию.
- `kMaxStringLenghtCsvSize = 1 МБ` — лимит длины ячейки CSV.
- `kYearPrefDays`, `kMonthPrefDays`, `kMonthPrefDaysLeap` — префиксные суммы дней.
- `kSecondsPerMinute / kSecondsPerHour / kSecondsPerDay`.

### `schema.h` / `schema.cpp`

Класс `Schema` ([`schema.h:7`](core/schema.h:7)) — описание набора колонок (имена + типы).

- `Schema()` — пустая.
- `Schema(std::fstream* schema, char delim=',', bool lf=true)` — чтение схемы из CSV (две колонки: имя/тип).
- `Schema(std::vector<std::string> names, std::vector<Types> types)` — из готовых векторов; бросает исключение, если размеры не совпадают.
- `const std::vector<std::string>& GetNames() const`.
- `const std::vector<Types>& GetTypes() const`.
- `size_t GetCntColumns() const`.
- `Types GetType(size_t idx) const` — с проверкой границ.
- `void SetTypes(const std::vector<Types>&)` — заменить только типы.

### `column.h` / `column.cpp`

Колонка — обёртка над `std::variant<std::vector<T>...>`.

- `struct Overloaded<Ts...>` ([`column.h:9`](core/column.h:9)) — стандартный helper для `std::visit`.
- `template<typename F> void DispatchColumnHelper(Types, F&&)` ([`column.h:17`](core/column.h:17)) — вызывает `F.template operator()<Types::...>()` для нужной ветки.
- Класс `Column` ([`column.h:57`](core/column.h:57)):
  - `Column()` / copy / move конструкторы.
  - `Column(const std::vector<T>&, Types)` / `Column(std::vector<T>&&, Types)` — из готового вектора.
  - `template<typename T> T GetElementByIndex(size_t)` — взять конкретное значение.
  - `Column GetElementByIndexAsColumn(size_t)` — однострочная колонка‑копия.
  - `void MergeWithOtherColumn(Column&&)` — конкатенация с проверкой типов.
  - `ColumnType& GetData() &` / `ColumnType&& GetData() &&` / `const ColumnType& GetData() const&` — доступ к variant.
  - `void TranslateTo(Types)` — преобразование всей колонки в другой тип.
  - `size_t GetSize() const`, `Types GetType() const`.
  - `void PrintCol()` — debug‑печать через glog.
- Свободные функции:
  - `int64_t WriteColToBzn(std::fstream*, Column&&)` — сериализация колонки. Возвращает количество записанных байт. `bool` бросает исключение.
  - `Column ReadColFromBzn(std::fstream*, Types, int64_t end)` — чтение в полу‑открытом диапазоне `[tellg, end)`.

### `batch.h` / `batch.cpp`

Батч — `Schema + std::vector<Column>`.

- `Batch()`.
- `Batch(Schema&&, std::vector<Column>&&)` / `Batch(const Schema&, std::vector<Column>&&)` — с проверкой соответствия числа колонок.
- `void NewSchema(Schema)` — заменить схему и привести типы (имена применяются только если новая схема непустая).
- `void NewTypes(std::vector<Types>)` — поменять только типы и сделать `TranslateTo`.
- `const Schema& GetSchema() const`.
- `size_t GetCntColumns() const` / `size_t GetColumnSize() const` — количество колонок и количество строк (по первой колонке).
- `Column& GetColumnIdx(size_t)` / `const Column& GetColumnIdx(size_t) const`.
- `const Column& GetColumnByName(const std::string&) const` — линейный поиск; бросает, если нет.
- `Batch GetRow(size_t)` — однострочный батч.
- `void MergeWithOtherBatch(Batch&&)` — построчная конкатенация (через `MergeWithOtherColumn`).
- `const std::vector<Column>& GetBatchData() const`.

### `csv-rw.h` / `csv-rw.cpp`

`CSVReader` ([`csv-rw.h:12`](io/csv-rw.h:12)) — потоковое чтение по RFC 4180 (`LF`/`CRLF`).

- `CSVReader(std::fstream* in, size_t cnt_columns, char delim=',', bool lf=true, bool have_header=false)`.
- `std::vector<std::string> GetRow()` — одна строка.
- `Batch GetBatch(size_t batch_row_size = kBatchRowSize)` — батч строк, тип всех колонок — `kString`.
- `bool IsReaded()` — конец файла.

`CSVWriter` ([`csv-rw.h:27`](io/csv-rw.h:27)):

- `CSVWriter(std::fstream*, bool lf=true)`.
- `void WriteBatch(Batch, char delim=',')` — приводит все колонки к строкам, экранирует кавычками. **Не удваивает внутренние кавычки** (см. [`review.md`](review.md)).

Внутренние:

- `PredicateLF` / `PredicateCRLF` — условие конца строки.
- `ScreenString(std::string&&)` — оборачивание в кавычки.

### `my-format.h` / `my-format.cpp`

`BZNReader` ([`my-format.h:11`](io/my-format.h:11)) — чтение BZN:

- `BZNReader(std::fstream*)` — читает meta‑префикс и схему.
- `Batch Read(const std::vector<std::string>& column_peek = {})` — следующий батч; если задан `column_peek`, остальные колонки пропускаются `seekg`.
- `bool IsReaded()` — пройден ли последний батч.
- `size_t GetCntColumns()`.
- (private) `void GetMetaOffset(int64_t file_end)`, `void BuildSchema()`, `std::vector<int64_t> GetMetaBatchOffset()`, `std::vector<std::string> GetMetaString()`.

`BZNWriter` ([`my-format.h:29`](io/my-format.h:29)) — запись BZN:

- `BZNWriter(Schema, std::fstream*)` — резервирует 8 байт под `meta_size`.
- `void Write(Batch)` — приводит батч к схеме, пишет колонки, обновляет таблицу смещений.
- `void WriteMetaInfo()` — финализация (имена/типы/смещения батчей + `meta_size`). Повторный вызов запрещён через флаг `locked_`.

### `expressions.h` / `expressions.cpp`

Дерево выражений уровня батча.

- `class IExpression` ([`expressions.h:7`](query/expressions.h:7)):
  - `virtual Column Evaluate(const Batch&) = 0`.
- `class ColumnRef : IExpression` ([`expressions.h:13`](query/expressions.h:13)):
  - `ColumnRef(const std::string& name)`.
  - `Column Evaluate(const Batch&)` — возвращает колонку батча по имени.
- `template<typename T> class Literal : IExpression` ([`expressions.h:24`](query/expressions.h:24)):
  - `Literal(T val, Types col_type)`.
  - `Column Evaluate(const Batch& b)` — `Column` той же длины, заполненный `val`.
- `enum class CmpType { L, Leq, Eq, G, Geq, Neq }`.
- `class BinaryCmp : IExpression` ([`expressions.h:40`](query/expressions.h:40)):
  - `BinaryCmp(left, CmpType, right)`.
  - `Column Evaluate(const Batch&)` — текущая реализация поддерживает только `Neq`, возвращает `vector<bool>` маску.
- `enum class FuncType { Sum }` и `class BinaryFunc : IExpression` — заглушка ([`expressions.h:57`](query/expressions.h:57)), реализация отсутствует.

### `operators.h` / `operators.cpp`

Pull‑based операторы; контракт — `std::optional<Batch> Next()` ([`operators.h:14`](query/operators.h:14)).

- `class ScanOperator` ([`operators.h:17`](query/operators.h:17)):
  - `ScanOperator(BZNReader&&, std::vector<std::string>&& columns)`.
  - `Next()` — читает очередной батч из BZN с проекцией колонок.
- `class FilterOperator` ([`operators.h:28`](query/operators.h:28)):
  - `FilterOperator(child, predicate_expression)`.
  - `Next()` — применяет предикат, фильтрует строки по `vector<bool>` маске.
- `enum class AggregateType { Sum, Count, CountDistinct, Min, Max, Avg }`.
- `class AggregateOperator` ([`operators.h:42`](query/operators.h:42)):
  - `AggregateOperator(child, std::vector<AggregateType>, std::vector<std::shared_ptr<IExpression>>)`.
  - `Next()` — материализует **весь** ввод, возвращает один батч с агрегатами.
- `class GroupByOperator` ([`operators.h:55`](query/operators.h:55)):
  - `GroupByOperator(child, aggregations, expressions, keys)`.
  - `Next()` — `std::map` по строковому ключу (склейка через `\x1F`), возвращает один батч `keys || aggregates`.
- `class OrderByLimitOperator` ([`operators.h:70`](query/operators.h:70)):
  - `OrderByLimitOperator(child, key_expression, limit)`.
  - `Next()` — top‑K через `std::multimap` (см. замечания в [`review.md`](review.md)).
- Свободные хелперы:
  - `FuncByAggregateTypeHelper(AggregateType, Types)` — фабрика `IAggregateFunc`.
  - `key_encoder(...)` / `key_decoder(...)` — сериализация составного ключа GroupBy.
- (приватный) `template<typename T> class LimitedSet` — мёртвый код.

### `aggregate.h` / `aggregate.cpp`

Базовые интерфейсы и реализации агрегатов.

- `class IAggregateState` — пустая база ([`aggregate.h:9`](query/aggregate.h:9)).
- `class IAggregateFunc` ([`aggregate.h:17`](query/aggregate.h:17)):
  - `virtual std::shared_ptr<IAggregateState> CreateState()`.
  - `virtual void Update(state, const Column&)`.
  - `virtual Column Finalize(state, Types)`.

Реализации (каждая — пара `*State` + `*Func`):

- `CountFunc` / `CountState` — счётчик строк, итог `int32` ([`aggregate.cpp:11`](query/aggregate.cpp:11)).
- `SumFunc` / `SumState<T>` — сумма; для int‑типов — в `int64`, для double/long double — в `long double` ([`aggregate.cpp:36`](query/aggregate.cpp:36)).
- `AvgFunc` / `AvgState` — среднее (`sum/cnt` в `long double`) ([`aggregate.cpp:114`](query/aggregate.cpp:114)).
- `CountDistinctFunc` / `CountDistinctState<T>` — `std::set<T>`, итог `int32` ([`aggregate.cpp:165`](query/aggregate.cpp:165)).
- `MinFunc` / `MinState<T>` — минимум, инициализация `numeric_limits<T>::max()` ([`aggregate.cpp:217`](query/aggregate.cpp:217)).
- `MaxFunc` / `MaxState<T>` — максимум, инициализация `numeric_limits<T>::min()` (баг для floating‑point, см. review) ([`aggregate.cpp:270`](query/aggregate.cpp:270)).

Каждая `*Func` имеет конструктор `*Func(Types out_type)` (кроме `CountFunc` и `AvgFunc`, которым тип не нужен).

### `CMakeLists.txt`

```cmake
add_library(engine_lib
    csv-rw.cpp my-format.cpp batch.cpp schema.cpp column.cpp
    types.cpp datatype.cpp expressions.cpp operators.cpp aggregate.cpp)
target_link_libraries(engine_lib PUBLIC glog::glog)
target_link_libraries(engine_lib PUBLIC Boost::boost)
target_include_directories(engine_lib PUBLIC .)
```

Цель `engine_lib` подключается из `exec/` и `tests/`.

---

## Лицензия и статус

Проект учебный (см. [`documents/coursework.pdf`](documents/coursework.pdf)). Текущий статус и известные ограничения — см. [`review.md`](review.md).
