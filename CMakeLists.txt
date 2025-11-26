cmake_minimum_required(VERSION 4.0)
project(arena VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TARGET_NAME ${PROJECT_NAME})
set(TEST_TARGET_NAME ${PROJECT_NAME}_test)
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(HEADERS
  "include/arena/vector.hpp")

set(GCC_OPTIONS -Wall -Wextra -Wpedantic -fmax-errors=1)
set(CLANG_OPTIONS -Wall -Wextra -Wpedantic -ferror-limit=1)

set(COMPILE_OPTIONS
  $<$<CXX_COMPILER_ID:GNU>:${GCC_OPTIONS}>
  $<$<CXX_COMPILER_ID:Clang>:${CLANG_OPTIONS}>)

find_package(doctest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
set(LIBRARIES doctest::doctest fmt::fmt)

add_executable(${TARGET_NAME} src/main.cpp)
target_sources(${TARGET_NAME} PRIVATE ${HEADERS})
target_include_directories(${TARGET_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_options(${TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(${TARGET_NAME} PRIVATE ${LIBRARIES})

include(CTest)
enable_testing()

add_executable(${TEST_TARGET_NAME} test/test.cpp)
target_sources(${TEST_TARGET_NAME} PRIVATE ${HEADERS})
target_include_directories(${TEST_TARGET_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_options(${TEST_TARGET_NAME} PRIVATE ${COMPILE_OPTIONS})
target_link_libraries(${TEST_TARGET_NAME} PRIVATE ${LIBRARIES})
add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})
