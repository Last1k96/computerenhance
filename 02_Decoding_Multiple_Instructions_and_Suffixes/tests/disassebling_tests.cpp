#include <gtest/gtest.h>
#include <ostream>

#include <decompile.h>
#include "utils.h"

namespace fs = std::filesystem;

static const std::string NASM = getShortPathName(NASM_EXEC);
static const std::string ASM_DIR = getShortPathName(ASM_LISTINGS_DIR);
static const std::string DISASM = getShortPathName(LESSON_EXE);
static const std::string WORK_BASE = getShortPathName(WORK_BASE_DIR);

struct InstructionDisasm : ::testing::TestWithParam<std::string> {
    void SetUp() override {
        spdlog::set_level(spdlog::level::debug);
    }

    std::optional<std::vector<uint8_t>> assembleWithNasm(std::string_view source) {
        auto workDir = fs::path(WORK_BASE);
        fs::create_directories(workDir);

        auto asmPath = workDir / "source.asm";
        std::ofstream{asmPath} << source;

        auto binPath = workDir / "assembled.bin";
        auto cmd = std::format(
                R"({} -f bin -o {} {})",
                getShortPathName(NASM),
                getShortPathName(binPath.string()),
                getShortPathName(asmPath.string())
        );
        if (std::system(cmd.c_str()) != 0) {
            spdlog::error("Failed to assemble the source");
            return std::nullopt;
        }

        std::ifstream in(binPath, std::ios::binary);
        auto data = std::vector<uint8_t>((std::istreambuf_iterator<char>(in)),
                                         std::istreambuf_iterator<char>());

        return data;
    }
};

TEST_P(InstructionDisasm, MovInstruction) {
    spdlog::set_level(spdlog::level::debug);

    auto snippet = GetParam();
    auto src = "bits 16\n" + snippet;

    auto binary = assembleWithNasm(src);
    ASSERT_TRUE(binary.has_value());

    auto disassembly = decompile(binary.value());
    ASSERT_TRUE(compareAsmLines(src, disassembly));
}

INSTANTIATE_TEST_SUITE_P(Lesson01, InstructionDisasm, ::testing::Values(
        // Simple mov tests from lesson 01
        "mov cx, bx",
        "mov ch, ah",
        "mov dx, bx",
        "mov si, bx",
        "mov bx, di",
        "mov al, cl",
        "mov ch, ch",
        "mov bx, ax",
        "mov bx, si",
        "mov sp, di",
        "mov bp, ax"
));

INSTANTIATE_TEST_SUITE_P(Complex, InstructionDisasm, ::testing::Values(
        // Register-to-register
        "mov si, bx",
        "mov dh, al",

        // 8-bit-immediate-to-register
        "mov cl, 12",
        "mov ch, -12",

        // 16-bit immediate-to-register
        "mov cx, 12",
        "mov cx, -12",
        "mov dx, 3948",
        "mov dx, -3948",

        // Source address calculation
        "mov al, [bx + si]",
        "mov bx, [bp + di]",
        "mov dx, [bp]",

        // Source address calculation plus 8-bit displacement
        "mov ah, [bx + si + 4]",

        // Source address calculation plus 16-bit displacement
        "mov al, [bx + si + 4999]",

        // Dest address calculation
        "mov [bx + di], cx",
        "mov [bp + si], cl",
        "mov [bp], ch"
));


INSTANTIATE_TEST_SUITE_P(ExtraComplex, InstructionDisasm, ::testing::Values(
        // Signed displacements
        "mov ax, [bx + di - 37]",
        "mov [si - 300], cx",
        "mov dx, [bx - 32]",

        // Explicit sizes
        "mov [bp + di], byte 7",
        "mov [di + 901], word 347",

        // Direct address
        "mov bp, [5]",
        "mov bx, [3458]",

        // Memory-to-accumulator test
        "mov ax, [2555]",
        "mov ax, [16]",

        // Accumulator-to-memory test
        "mov [2554], ax",
        "mov [15], ax"
));