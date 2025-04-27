cmake_minimum_required(VERSION 3.30)
project(computer_enhance LANGUAGES CXX ASM_NASM)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        11.1.4
)

# This will clone fmt and add it as a subdirectory
FetchContent_MakeAvailable(fmt)

add_subdirectory("01_Instruction_Decoding_on_the_8086")
