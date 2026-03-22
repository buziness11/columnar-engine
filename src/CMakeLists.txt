add_library(engine_lib csv-rw.cpp my-format.cpp batch.cpp schema.cpp column.cpp types.cpp datetype.cpp)
target_link_libraries(engine_lib PUBLIC glog::glog)
target_include_directories(engine_lib PUBLIC .)
