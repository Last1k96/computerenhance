cmake_minimum_required(VERSION 3.30)
project(01_lesson LANGUAGES CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(ASM_NASM)
set(NASM ${CMAKE_ASM_NASM_COMPILER})

#
# Glob and compile the listing asm files
#

file(GLOB ASM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/listings/*.asm")
if (NOT ASM_SOURCES)
    message(FATAL_ERROR "No .asm files found in listings/")
endif ()

#
# 01_disassembler header target
#

add_library(01_disassembler INTERFACE)
target_include_directories(01_disassembler
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(01_disassembler
        INTERFACE
        spdlog::spdlog
)

#
# 01_disassembler executable
#

add_executable(01_lesson main.cpp)
target_link_libraries(01_lesson
        PRIVATE
        01_disassembler
        fmt::fmt
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(01_lesson
            PRIVATE
            -static
            -static-libstdc++
            -static-libgcc
    )
endif()

#
# Google test
#

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(01_disasm_tests
        tests/listing_tests.cpp
)

target_include_directories(01_disasm_tests PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_dependencies(01_disasm_tests
        01_lesson
)

target_compile_definitions(01_disasm_tests
        PRIVATE
        NASM_EXEC="${NASM}"
        ASM_LISTINGS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/listings"
        LESSON_EXE="$<TARGET_FILE:01_lesson>"
        WORK_BASE_DIR="${CMAKE_CURRENT_BINARY_DIR}/01_disasm_tests"
)

target_link_libraries(01_disasm_tests
        PRIVATE
        gtest_main
        01_disassembler
)

