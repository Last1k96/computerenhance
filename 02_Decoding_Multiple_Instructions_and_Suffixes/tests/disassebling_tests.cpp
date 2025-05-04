#include <gtest/gtest.h>
#include <ostream>

#include <decompile.h>
#include "utils.h"

namespace fs = std::filesystem;

static const std::string NASM = getShortPathName(NASM_EXEC);
static const std::string ASM_DIR = getShortPathName(ASM_LISTINGS_DIR);
static const std::string DISASM = getShortPathName(LESSON_EXE);
static const std::string WORK_BASE = getShortPathName(WORK_BASE_DIR);

struct InstructionDisasm : ::testing::Test {
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

        spdlog::debug("Assembled {} into {}", asmPath.string(), binPath.string());

        std::ifstream in(binPath, std::ios::binary);
        auto data = std::vector<uint8_t>((std::istreambuf_iterator<char>(in)),
                                         std::istreambuf_iterator<char>());

        return data;
    }
};

TEST_F(InstructionDisasm, RegisterToRegister) {
    spdlog::set_level(spdlog::level::info);

    auto src = "bits 16\nmov si, bx";

    auto binary = assembleWithNasm(src);
    ASSERT_TRUE(binary.has_value());

    auto disassembly = decompile(binary.value());
    ASSERT_EQ(compareAsmLines(src, disassembly), 0);
}