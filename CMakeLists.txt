cmake_minimum_required(VERSION 3.10)
project(PersonalFinanceManager CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra)
endif()

include_directories(include)

file(GLOB APP_SOURCES src/*.cpp)
add_executable(PersonalFinanceManager ${APP_SOURCES})

# Tests: all sources except main.cpp, plus tests/*.cpp
set(LIB_SOURCES ${APP_SOURCES})
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
add_executable(run_tests tests/test_core.cpp ${LIB_SOURCES})
