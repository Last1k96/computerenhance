cmake_minimum_required(VERSION 3.30)
project(computer_enhance LANGUAGES CXX ASM_NASM)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        11.1.4
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory("01_Instruction_Decoding_on_the_8086")
