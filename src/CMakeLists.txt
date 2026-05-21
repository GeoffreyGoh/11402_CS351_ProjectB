cmake_minimum_required(VERSION 3.10)

project(CSVMiniDB CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(csv_minidb src/main.cpp)
add_executable(db_tests src/tests.cpp)