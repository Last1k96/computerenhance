cmake_minimum_required(VERSION 3.30)
project(02_lesson LANGUAGES CXX ASM_NASM)

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
# Disassembler header target
#

add_library(disassembler INTERFACE)
target_include_directories(disassembler
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(disassembler
        INTERFACE
        spdlog::spdlog
)

#
# Disassembler executable
#

add_executable(02_lesson main.cpp)
target_link_libraries(02_lesson
        PRIVATE
        disassembler
        fmt::fmt
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(02_lesson
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

add_executable(02_disasm_tests
        tests/listing_tests.cpp
        tests/disassebling_tests.cpp
        tests/utils.h
)

target_include_directories(02_disasm_tests PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_dependencies(02_disasm_tests
        02_lesson
)

target_compile_definitions(02_disasm_tests
        PRIVATE
        NASM_EXEC="${NASM}"
        ASM_LISTINGS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/listings"
        LESSON_EXE="$<TARGET_FILE:02_lesson>"
        WORK_BASE_DIR="${CMAKE_CURRENT_BINARY_DIR}/02_disasm_tests"
)

target_link_libraries(02_disasm_tests
        PRIVATE
        gtest_main
        disassembler
)

