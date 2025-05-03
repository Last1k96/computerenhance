#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <format>
#include <decompile.h>

#ifdef _WIN32

#include <windows.h>

#endif

namespace fs = std::filesystem;

static const std::string NASM = NASM_EXEC;
static const std::string ASM_DIR = ASM_LISTINGS_DIR;
static const std::string DISASM = LESSON_EXE;
static const std::string WORK_BASE = WORK_BASE_DIR;

std::string getShortPathName(const std::string &longPath) {
#ifdef _WIN32
    // Get the required buffer size
    DWORD length = GetShortPathNameA(longPath.c_str(), NULL, 0);
    if (length == 0) return longPath; // Error, return original

    // Allocate buffer
    std::vector<char> buffer(length);

    // Get the short path
    DWORD result = GetShortPathNameA(longPath.c_str(), buffer.data(), length);
    if (result == 0) return longPath; // Error, return original

    return buffer.data();
#else
    return longPath; // Non-Windows, return original
#endif
}

// helper to read a file into memory
static std::vector<char> read_bytes(const fs::path &p) {
    std::ifstream in(p, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

std::string slurp_file(const std::filesystem::path &p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open " + p.string());

    // Construct string from istreambuf_iterators:
    return {std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>()};
}


// gather all .asm files under ASM_DIR
static std::vector<fs::path> collect_listings() {
    std::vector<fs::path> v;
    for (auto &ent: fs::directory_iterator(ASM_DIR)) {
        if (ent.path().extension() == ".asm")
            v.push_back(ent.path());
    }
    return v;
}

// … all your includes, namespace aliases, helper functions, etc. …

struct DisasmTest : ::testing::TestWithParam<fs::path> {
    fs::path src;
    fs::path workDir;
    fs::path origBin;
    fs::path outAsm;
    fs::path recompBin;

    void SetUp() override {
        src = GetParam();
        workDir = fs::path(WORK_BASE) / src.stem();
        fs::create_directories(workDir);

        origBin = workDir / "orig.bin";
        outAsm = workDir / "out.asm";
        recompBin = workDir / "recomp.bin";

        spdlog::set_level(spdlog::level::debug);
    }

    void AssembleOriginal() {
        std::string cmd = std::format(
                R"({} -f bin -o {} {})",
                getShortPathName(NASM),
                getShortPathName(origBin.string()),
                getShortPathName(src.string())
        );
        ASSERT_EQ(std::system(cmd.c_str()), 0)
                                    << "NASM failed on " << src;
    }

    void DisassembleOrigWithSource() {
        auto binaryData = readFile(origBin.string());
        auto source = decompile(binaryData);

        std::ofstream ofs(outAsm.string());
        ASSERT_TRUE(ofs) << "Error: could not open file for writing: " << outAsm.string();

        ofs << source;
    }

    void DisassembleOrigWithBinary() {
        std::string cmd = std::format(
                R"({} {} > {})",
                getShortPathName(DISASM),
                getShortPathName(origBin.string()),
                getShortPathName(outAsm.string())
        );
        ASSERT_EQ(std::system(cmd.c_str()), 0)
                                    << "Failed to decompile the binary";
    }

    void ReassembleOut() {
        std::string cmd = std::format(
                R"({} -f bin -o {} {})",
                getShortPathName(NASM),
                getShortPathName(recompBin.string()),
                getShortPathName(outAsm.string())
        );
        ASSERT_EQ(std::system(cmd.c_str()), 0)
                                    << "NASM failed on reconstructed asm";
    }
};

TEST_P(DisasmTest, AssembleOriginal) {
    AssembleOriginal();
}

TEST_P(DisasmTest, Disassemble) {
    AssembleOriginal();
    DisassembleOrigWithSource();
}

TEST_P(DisasmTest, Reassemble) {
    AssembleOriginal();
    DisassembleOrigWithSource();
    ReassembleOut();
}

TEST_P(DisasmTest, CompareBinariesDebugSource) {
    AssembleOriginal();
    DisassembleOrigWithSource();
    ReassembleOut();

    auto a = read_bytes(origBin);
    auto b = read_bytes(recompBin);
    auto disassembledAsm = slurp_file(outAsm);
    auto referenceAsm = slurp_file(src);
    EXPECT_EQ(a, b) << "Disassembled asm: \n" << disassembledAsm << "\n\nReference asm: \n" << referenceAsm << "\n";
}

TEST_P(DisasmTest, CompareBinariesDebugApplication) {
    AssembleOriginal();
    DisassembleOrigWithBinary();
    ReassembleOut();

    auto a = read_bytes(origBin);
    auto b = read_bytes(recompBin);
    auto disassembledAsm = slurp_file(outAsm);
    auto referenceAsm = slurp_file(src);
    EXPECT_EQ(a, b) << "Disassembled asm: \n" << disassembledAsm << "\n\nReference asm: \n" << referenceAsm << "\n";
}

INSTANTIATE_TEST_SUITE_P(
        Listings,
        DisasmTest,
        ::testing::ValuesIn(collect_listings())
);
