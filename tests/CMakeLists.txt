add_executable(unit_tests test_csv.cpp)
target_link_libraries(unit_tests PRIVATE engine_lib GTest::gtest_main)

add_test(NAME CSVReaderTests COMMAND unit_tests)
