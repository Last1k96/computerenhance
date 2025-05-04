// Lesson 02: Decoding Multiple Instructions and Suffixes
// https://www.computerenhance.com/p/decoding-multiple-instructions-and

#include <decompile.h>

int main(int argc, char* argv[]) {
    // disable debug logging
    spdlog::set_level(spdlog::level::off);

    auto filePath = parseArgs(argc, argv);
    if (!filePath.has_value()) {
        return 1;
    }
    auto binaryData = readFile(filePath.value());
    auto source = decompile(binaryData);

    std::cout << source;

    return 0;
}
