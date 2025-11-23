cmake_minimum_required(VERSION 3.20)
project(arena VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TARGET_NAME ${PROJECT_NAME})
set(TEST_TARGET_NAME ${PROJECT_NAME}_test)

add_executable(${TARGET_NAME} src/main.cpp)

file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.hpp")
if(PROJECT_HEADERS)
    target_sources(${TARGET_NAME} PRIVATE ${PROJECT_HEADERS})
endif()

target_compile_options(${TARGET_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)
target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE fmt::fmt)

include(CTest)
enable_testing()

add_executable(${TEST_TARGET_NAME} test/test.cpp)
target_compile_options(${TEST_TARGET_NAME}  PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

target_include_directories(${TEST_TARGET_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/test/*.hpp")

if(TEST_HEADERS)
    target_sources(${TEST_TARGET_NAME} PRIVATE ${TEST_HEADERS})
endif()

add_test(NAME ${TEST_TARGET_NAME} COMMAND ${TEST_TARGET_NAME})
