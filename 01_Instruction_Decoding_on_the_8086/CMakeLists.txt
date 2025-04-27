cmake_minimum_required(VERSION 3.30)
project(lesson_01 LANGUAGES CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(ASM_NASM)
set(NASM ${CMAKE_ASM_NASM_COMPILER})

# ── gather *.asm files ────────────────────────────────────────────────
file(GLOB ASM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/listings/*.asm")
if (NOT ASM_SOURCES)
    message(FATAL_ERROR "No .asm files found in listings/")
endif ()

# ── one custom rule per .asm ───────────────────────────────────────────
set(ASM_BINS)
foreach (src IN LISTS ASM_SOURCES)
    get_filename_component(fname "${src}" NAME_WE)
    set(out_dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    set(bin "${out_dir}/${fname}.bin")

    add_custom_command(
            OUTPUT ${bin}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${out_dir}"
            COMMAND ${NASM} -f bin -o ${bin} ${src}
            DEPENDS ${src}
            COMMENT "Assembling ${src}"
            VERBATIM
    )
    list(APPEND ASM_BINS ${bin})
endforeach ()

add_custom_target(assemble_listings ALL DEPENDS ${ASM_BINS})

# ── your disassembler executable ────────────────────────────────────────
add_executable(lesson_01 main.cpp)
add_dependencies(lesson_01 assemble_listings)
target_link_libraries(lesson_01
        PRIVATE
        fmt::fmt
)

# ── Fetch GoogleTest via FetchContent ─────────────────────────────────
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
)
# this both downloads and adds gtest & gmock via add_subdirectory()
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(disasm_tests
        tests/disasm_test.cpp
)

add_dependencies(disasm_tests
        lesson_01
        assemble_listings
)

# pass each DEFINE as a single token (no spaces around '=')
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
)

add_test(NAME RoundTripDisasm COMMAND disasm_tests)
