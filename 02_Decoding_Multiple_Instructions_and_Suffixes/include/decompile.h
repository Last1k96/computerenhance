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
#include <ranges>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

template<std::ranges::range R>
auto enumerate(R &&r) {
    auto indices = std::views::iota(std::size_t{0}, std::ranges::size(r));
    return std::views::zip(indices, std::forward<R>(r));
}

static std::vector<uint8_t> readFile(const std::filesystem::path &p) {
    auto sz = std::filesystem::file_size(p);

    std::vector<uint8_t> buf(sz);

    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Unable to open " + p.string());
    in.read(reinterpret_cast<char *>(buf.data()), sz);
    if (!in) throw std::runtime_error("Error reading " + p.string());

    return buf;
}

static std::optional<fs::path> parseArgs(int argc, char *argv[]) {
    if (argc != 2) {
        spdlog::error("Usage: {} <input-file-path>", fs::path{argv[0]}.filename().string());
        return std::nullopt;
    }

    std::string_view raw{argv[1]};
    fs::path input_path{raw};

    if (!fs::exists(input_path) || !fs::is_regular_file(input_path)) {
        spdlog::error("Error: {} is not a regular file", input_path.string());
        return std::nullopt;
    }

    return input_path;
}

static std::string_view decodeRegister(uint8_t bits, bool W) {
    assert((bits >> 3) == 0);

    switch (bits) {
        case 0b000:
            return W ? "ax" : "al";
        case 0b001:
            return W ? "cx" : "cl";
        case 0b010:
            return W ? "dx" : "dl";
        case 0b011:
            return W ? "bx" : "bl";
        case 0b100:
            return W ? "sp" : "ah";
        case 0b101:
            return W ? "bp" : "ch";
        case 0b110:
            return W ? "si" : "dh";
        case 0b111:
            return W ? "di" : "bh";
        default:
            spdlog::error("Failed to decode register {:08b} with W flag = {}", bits, W ? "true" : "false");
            assert(false && "Failed to decode register");
    }
}

// Follow Intel-8086 user manual, page 261, section 4-18
static std::string decompile(const std::vector<uint8_t> &binaryData) {
    spdlog::debug("Decompiling binary: {} bytes", binaryData.size());

    std::string decodedInstructions;
    decodedInstructions.append("bits 16\n");

    auto D = 0;
    auto W = 0;

    for (int64_t i = 0; i < static_cast<int64_t>(binaryData.size()); i++) {
        auto byte = binaryData[i];
        spdlog::debug("byte {}: {:08b}", i, byte);

        if ((byte & ~0b11) == 0b10001000) {
            // Register/memory to/from register
            D = (byte >> 1) & 1;
            W = byte & 1;

            spdlog::debug("mov (D={} W={})", D, W);
            decodedInstructions.append("mov ");
        } else if ((byte & ~0b1) == 0b11000110){
            // Immediate to register/memory
            spdlog::error("Immediate to register/memory MOV is not implemented");
            break;
        } else if ((byte & ~0b1111) == 0b10110000) {
            // Immediate to register
            spdlog::error("Immediate to register MOV is not implemented");
            break;
        } else if ((byte & ~0b1) == 0b10100000) {
            // Memory to accumulator
            spdlog::error("Memory to accumulator MOV is not implemented");
            break;
        } else if ((byte & ~0b1) == 0b10100010) {
            // Accumulator to memory
            spdlog::error("Accumulator to memory MOV is not implemented");
            break;
        } else {
            spdlog::error("Failed to recognize instruction: {:08b}", byte);
            break;
        }

        i++;
        byte = binaryData[i];

        auto mod = (byte >> 6);
        auto reg = (byte >> 3) & 0b111;
        auto rm = byte & 0b111;

        assert(mod == 0b11 && "Only Register Mode is implemented");

        auto reg0 = decodeRegister(reg, W);
        auto reg1 = decodeRegister(rm, W);

        if (D == 1) {
            std::swap(reg0, reg1); // Instruction destination specified in REG field
        }

        auto decoded = std::format("{}, {}\n", reg1, reg0);
        spdlog::debug("{}", decoded);

        decodedInstructions.append(decoded);
    }

    return decodedInstructions;
}