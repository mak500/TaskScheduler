cmake_minimum_required (VERSION 3.8)

project ("ThreadPool")

set (CMAKE_CXX_STANDARD 20)

add_executable(main main.cc)
target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/include)
