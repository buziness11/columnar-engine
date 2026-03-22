add_executable(test_csv test_csv.cpp) add_executable(
    test_format test_format.cpp) add_executable(bench_rw_hits bench_rw_hits.cpp)

    target_link_libraries(test_csv PRIVATE engine_lib GTest::gtest_main)
        target_link_libraries(test_format PRIVATE engine_lib GTest::gtest_main)
            target_link_libraries(
                bench_rw_hits PRIVATE engine_lib GTest::gtest_main)

                add_test(NAME CSVReaderTests COMMAND test_csv)
                    add_test(NAME MyFormatTests COMMAND test_format)
                        add_test(NAME BenchTests COMMAND bench_rw_hits)

                            target_compile_definitions(
                                test_csv PRIVATE TEST_DATA_DIR =
                                    "${CMAKE_CURRENT_SOURCE_DIR}/data/")

                                target_compile_definitions(
                                    test_format PRIVATE TEST_DATA_DIR =
                                        "${CMAKE_CURRENT_SOURCE_DIR}/data/")

                                    target_compile_definitions(
                                        bench_rw_hits PRIVATE TEST_DATA_DIR =
                                            "${CMAKE_CURRENT_SOURCE_DIR}/data/")
