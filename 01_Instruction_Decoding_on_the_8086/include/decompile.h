#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <span>
#include <stdexcept>
#include <bitset>
#include <iostream>
#include <optional>
#include <format>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

std::vector<std::byte> readFile(const std::filesystem::path& p) {
    auto sz = std::filesystem::file_size(p);

    std::vector<std::byte> buf(sz);

    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Unable to open " + p.string());
    in.read(reinterpret_cast<char*>(buf.data()), sz);
    if (!in) throw std::runtime_error("Error reading " + p.string());

    return buf;
}

std::optional<fs::path> parseArgs(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr
                << "Usage: "
                << fs::path{argv[0]}.filename().string()
                << " <input-file-path>\n";
        return std::nullopt;
    }

    // 1) Wrap argv[1] in a non-owning view:
    std::string_view raw{argv[1]};

    // 2) Build a filesystem::path from it:
    fs::path input_path{raw};

    // Optional: validate
    if (!fs::exists(input_path) || !fs::is_regular_file(input_path)) {
        std::cerr << "Error: “" << input_path << "” is not a regular file\n";
        return std::nullopt;
    }

    return std::nullopt;
}

std::string decompile(const std::vector<std::byte>& binaryData) {
    std::string s;

    for (auto byte : binaryData) {
        auto raw = std::to_integer<unsigned>(byte);
        std::bitset<8> bits(raw);
        //std::cout << bits << "\n";
        // bits[7] is the MSB, bits[0] is the LSB
    }

    s.append("bits 16\n");
    return s;
}