cmake_minimum_required(VERSION 3.30)

# Expected variables ----------------------------------------------------
#   ASM_SRC      – full path of the original .asm file
#   NASM         – path to the NASM executable
#   LESSON_EXE   – path to the disassembler program (lesson_01)
#   WORK_DIR     – directory where we do all scratch work
# ----------------------------------------------------------------------

file(MAKE_DIRECTORY "${WORK_DIR}")

# 1. assemble source  →  orig.bin
set(ORIG_BIN "${WORK_DIR}/orig.bin")
execute_process(
        COMMAND "${NASM}" -f bin -o "${ORIG_BIN}" "${ASM_SRC}"
        RESULT_VARIABLE rc
        COMMAND_ERROR_IS_FATAL ANY
)

# 2. disassemble orig.bin  →  out.asm
set(OUT_ASM  "${WORK_DIR}/out.asm")
execute_process(
        COMMAND "${LESSON_EXE}" "${ORIG_BIN}" "${OUT_ASM}"
        RESULT_VARIABLE rc
        COMMAND_ERROR_IS_FATAL ANY
)

# 3. assemble out.asm  →  recomp.bin
set(RECOMP_BIN "${WORK_DIR}/recomp.bin")
execute_process(
        COMMAND "${NASM}" -f bin -o "${RECOMP_BIN}" "${OUT_ASM}"
        RESULT_VARIABLE rc
        COMMAND_ERROR_IS_FATAL ANY
)

# 4. compare the two binaries (non-zero result -> test failure)
execute_process(
        COMMAND "${CMAKE_COMMAND}" -E compare_files "${ORIG_BIN}" "${RECOMP_BIN}"
        RESULT_VARIABLE cmp_rc
)
if (cmp_rc)
    message(FATAL_ERROR "Round-trip mismatch: ${ASM_SRC}")
endif()