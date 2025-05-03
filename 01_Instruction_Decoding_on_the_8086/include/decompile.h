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

std::vector<std::byte> readFile(const std::filesystem::path &p) {
    auto sz = std::filesystem::file_size(p);

    std::vector<std::byte> buf(sz);

    std::ifstream in(p, std::ios::binary);
    if (!in) throw std::runtime_error("Unable to open " + p.string());
    in.read(reinterpret_cast<char *>(buf.data()), sz);
    if (!in) throw std::runtime_error("Error reading " + p.string());

    return buf;
}

std::optional<fs::path> parseArgs(int argc, char *argv[]) {
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

std::string_view decodeRegister(uint8_t bits, bool W) {
    assert((bits >> 3) == 0);

    switch (bits) {
        case 0b000:
            return W ? "AX" : "AL";
        case 0b001:
            return W ? "CX" : "CL";
        case 0b010:
            return W ? "DX" : "DL";
        case 0b011:
            return W ? "BX" : "BL";
        case 0b100:
            return W ? "SP" : "AH";
        case 0b101:
            return W ? "BP" : "CH";
        case 0b110:
            return W ? "SI" : "DH";
        case 0b111:
            return W ? "DI" : "BH";
        default:
            spdlog::error("Failed to decode register {:08b} with W flag = {}", bits, W ? "true" : "false");
            assert(false && "Failed to decode register");
    }
}

enum class DECODING_STATE {
    INITIAL,
    MOV,
};

// Follow Intel-8086 user manual, page 261, section 4-18
std::string decompile(const std::vector<std::byte> &binaryData) {
    spdlog::debug("Decompiling binary: {} bytes", binaryData.size());

    std::string decodedInstructions;
    decodedInstructions.append("bits 16\n");

    auto decodingState = DECODING_STATE::INITIAL;
    auto D = 0;
    auto W = 0;

    for (auto [i, byte]: enumerate(binaryData)) {
        spdlog::debug("bit {}: {:08b}", i, byte);
        auto bits = std::to_integer<uint8_t>(byte);

        switch (decodingState) {
            case DECODING_STATE::INITIAL: {
                auto instructionBits = (bits >> 2);

                if (instructionBits == 0b00100010) {
                    decodingState = DECODING_STATE::MOV;
                    D = (bits >> 1) & 1;
                    W = bits & 1;

                    spdlog::debug("MOV (D={} W={})", D, W);
                    decodedInstructions.append("MOV ");
                    continue;
                } else {
                    spdlog::error("Failed to decode instruction {:08b} at {}", byte, i);
                }
                break;
            }

            case DECODING_STATE::MOV: {
                auto mod = (bits >> 6);
                auto reg = (bits >> 3) & 0b111;
                auto rm = bits & 0b111;

                assert(mod == 0b11 && "Only Register Mode is implemented");

                auto reg0 = decodeRegister(reg, W);
                auto reg1 = decodeRegister(rm, W);

                if (D == 1) {
                    std::swap(reg0, reg1); // Instruction destination specified in REG field
                }

                auto decoded = std::format("{}, {}\n", reg1, reg0);
                spdlog::debug("{}", decoded);

                decodedInstructions.append(decoded);
                decodingState = DECODING_STATE::INITIAL;
                break;
            }
        }
    }

    return decodedInstructions;
}