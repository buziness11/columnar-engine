add_library(engine_lib csv-reader.cpp)
target_link_libraries(engine_lib PUBLIC glog::glog)
target_include_directories(engine_lib PUBLIC .)