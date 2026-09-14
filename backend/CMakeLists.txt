cmake_minimum_required(VERSION 3.24)

project(codeplag VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    cpp_httplib
    # v0.18.x remains compatible with the local MinGW toolchain. Newer releases
    # intentionally stopped supporting MinGW, while this project also builds on
    # the supported Linux/GCC path used by its Docker image.
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(cpp_httplib nlohmann_json)

add_library(codeplag_core
    src/compare.cpp
    src/hashing.cpp
    src/tokenizer.cpp
    src/winnow.cpp
)
target_include_directories(codeplag_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(codeplag_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(codeplag_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(codeplag_server src/main.cpp)
target_link_libraries(
    codeplag_server
    PRIVATE codeplag_core httplib::httplib nlohmann_json::nlohmann_json
)

if(MSVC)
    target_compile_options(codeplag_server PRIVATE /W4 /permissive-)
else()
    target_compile_options(codeplag_server PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(CODEPLAG_BUILD_TESTS "Build the backend test suite" ON)

if(CODEPLAG_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        Catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(codeplag_tests
        tests/test_compare.cpp
        tests/test_hashing.cpp
        tests/test_tokenizer.cpp
        tests/test_winnow.cpp
    )
    target_link_libraries(codeplag_tests PRIVATE codeplag_core Catch2::Catch2WithMain)

    include(Catch)
    catch_discover_tests(codeplag_tests)
endif()
