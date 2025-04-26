cmake_minimum_required(VERSION 3.30)
project(lesson_01 CXX ASM_NASM)

enable_language(ASM_NASM)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)

file(GLOB ASM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/listings/*.asm")

if(NOT ASM_SOURCES)
    message(FATAL_ERROR "No .asm files found!")
endif()

set(ASM_BINS)
foreach(src IN LISTS ASM_SOURCES)
    get_filename_component(fname "${src}" NAME_WE)
    set(bin "${CMAKE_CURRENT_BINARY_DIR}/${fname}.bin")

    add_custom_command(
            OUTPUT  ${bin}
            COMMAND nasm -f bin -o ${bin} ${src}
            DEPENDS ${src}
            COMMENT "Assembling ${src}"
            VERBATIM)

    list(APPEND ASM_BINS ${bin})
endforeach()

add_custom_target(assemble_listings ALL DEPENDS ${ASM_BINS})

add_executable(lesson_01 main.cpp)
add_dependencies(lesson_01 assemble_listings)
