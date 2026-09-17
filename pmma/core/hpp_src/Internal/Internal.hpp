#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace PMMA {
class Display;
}

namespace PMMA::Internal {
struct ColorEntry {
    std::string_view name;
    std::array<uint8_t, 3> value;
};

uint32_t GetRandomSeed();

std::optional<std::array<uint8_t, 3>> FindColor(std::string_view key);

void DisplayExistsCheck(PMMA::Display *Display);

} // namespace PMMA::Internal