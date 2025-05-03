cmake_minimum_required(VERSION 3.30)
project(lesson_01 LANGUAGES CXX ASM_NASM)

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

add_executable(lesson_01 main.cpp)
target_link_libraries(lesson_01
        PRIVATE
        disassembler
        fmt::fmt
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_options(lesson_01
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

add_executable(disasm_tests
        tests/disasm_test.cpp
)

target_include_directories(disasm_tests PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_dependencies(disasm_tests
        lesson_01
)

target_compile_definitions(disasm_tests
        PRIVATE
        NASM_EXEC="${NASM}"
        ASM_LISTINGS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/listings"
        LESSON_EXE="$<TARGET_FILE:lesson_01>"
        WORK_BASE_DIR="${CMAKE_CURRENT_BINARY_DIR}/disasm_tests"
)

target_link_libraries(disasm_tests
        PRIVATE
        gtest_main
        disassembler
)

