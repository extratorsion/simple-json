include(GoogleTest)
include(FetchContent)

cmake_minimum_required(VERSION 3.11)
project(simple_json LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories(src)
aux_source_directory(src SRC_FILES)

fetchcontent_declare(googletest
    URL https://codeload.github.com/google/googletest/zip/release-1.8.0
)

if (NOT googletest_POPULATED)
    fetchcontent_populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

# fetchcontent_makeavaliable(googletest)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(CMAKE_BUILD_TYPE "Debug")

add_executable(
    json-test
    tests/json-tests.cc
    ${SRC_FILES}
)

target_link_libraries(
    json-test
    gtest_main
)

enable_testing()
gtest_discover_tests(json-test)