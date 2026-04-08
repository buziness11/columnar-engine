set(EXECS exec_queries columnar_to_csv csv_to_columnar get_col_from_bzn)

foreach(EXE ${EXECS})
    add_executable(${EXE} ${EXE}.cpp)
    target_link_libraries(${EXE} PUBLIC glog::glog)
    target_include_directories(${EXE} PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(${EXE} PRIVATE 
        engine_lib
    )
    target_compile_definitions(${EXE} PRIVATE 
        TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/data/"
    )
endforeach()