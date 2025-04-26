#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <format>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

static const std::string NASM      = NASM_EXEC;
static const std::string ASM_DIR   = ASM_LISTINGS_DIR;
static const std::string DISASM    = LESSON_EXE;
static const std::string WORK_BASE = WORK_BASE_DIR;

std::string getShortPathName(const std::string& longPath) {
#ifdef _WIN32
    // Get the required buffer size
    DWORD length = GetShortPathNameA(longPath.c_str(), NULL, 0);
    if (length == 0) return longPath; // Error, return original

    // Allocate buffer
    std::vector<char> buffer(length);

    // Get the short path
    DWORD result = GetShortPathNameA(longPath.c_str(), buffer.data(), length);
    if (result == 0) return longPath; // Error, return original

    return std::string(buffer.data());
#else
    return longPath; // Non-Windows, return original
#endif
}

// helper to read a file into memory
static std::vector<char> read_bytes(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    return { std::istreambuf_iterator<char>(in), {} };
}

// gather all .asm files under ASM_DIR
static std::vector<fs::path> collect_listings() {
    std::vector<fs::path> v;
    for (auto& ent : fs::directory_iterator(ASM_DIR)) {
        if (ent.path().extension() == ".asm")
            v.push_back(ent.path());
    }
    return v;
}

struct DisasmTest : ::testing::TestWithParam<fs::path> {};

TEST_P(DisasmTest, RoundTripBinary) {
    fs::path src      = GetParam();
    fs::path workDir  = fs::path(WORK_BASE) / src.stem();
    fs::create_directories(workDir);

    fs::path origBin   = workDir / "orig.bin";
    fs::path outAsm    = workDir / "out.asm";
    fs::path recompBin = workDir / "recomp.bin";

    // 1) assemble original .asm
    {
        std::string cmd = std::format(
                R"({} -f bin -o {} {})",
                getShortPathName(NASM),
                getShortPathName(origBin.string()),
                getShortPathName(src.string())
        );

        std::cout << cmd << "\n";
        ASSERT_EQ(std::system(cmd.c_str()), 0)
                                    << "NASM failed on " << src;
    }
    return;
    // 2) run your disassembler, capturing stdout → out.asm
    {
        std::string cmd = "\"" + DISASM + "\" \"" +
                          origBin.string() + "\" > \"" +
                          outAsm.string() + "\"";
        ASSERT_EQ(std::system(cmd.c_str()), 0)
                                    << "Disassembler failed on " << origBin;
    }

    // 3) re-assemble the generated .asm
    {
        std::string cmd = "\"" + NASM + "\" -f bin -o \"" +
                          recompBin.string() + "\" \"" +
                          outAsm.string() + "\"";
        ASSERT_EQ(std::system(cmd.c_str()), 0)
                                    << "NASM failed on reconstructed asm";
    }

    // 4) compare binaries
    auto a = read_bytes(origBin);
    auto b = read_bytes(recompBin);
    EXPECT_EQ(a, b) << "Binary mismatch for " << src;
}

INSTANTIATE_TEST_SUITE_P(
        Listings,
        DisasmTest,
        ::testing::ValuesIn(collect_listings())
);
