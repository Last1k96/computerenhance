cmake_minimum_required(VERSION 3.30)
project(computer_enhance LANGUAGES CXX ASM_NASM)

# ── Fetch GoogleTest via FetchContent ─────────────────────────────────
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        main
)
# this both downloads and adds gtest & gmock via add_subdirectory()
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory("01_Instruction_Decoding_on_the_8086")
