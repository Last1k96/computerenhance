// Lesson 01: Instruction Decoding on the 8086
// https://www.computerenhance.com/p/instruction-decoding-on-the-8086

#include <decompile.h>

int main(int argc, char* argv[]) {
    auto filePath = parseArgs(argc, argv);
    if (!filePath.has_value()) {
        return 1;
    }
    auto binaryData = readFile(filePath.value());
    auto source = decompile(binaryData);

    std::cout << source;

    return 0;
}
