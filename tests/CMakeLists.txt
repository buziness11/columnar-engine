add_executable(csv_tests test_csv.cpp)
add_executable(transform_tests test_format.cpp)
target_link_libraries(csv_tests PRIVATE engine_lib GTest::gtest_main)
target_link_libraries(transform_tests PRIVATE engine_lib GTest::gtest_main)

add_test(NAME CSVReaderTests COMMAND csv_tests)
add_test(NAME MyFormatTests COMMAND transform_tests)

target_compile_definitions(csv_tests PRIVATE 
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/"
)

target_compile_definitions(transform_tests PRIVATE 
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/"
)