#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

#include <bgfx/bgfx.h>

#include "Internal/Internal.hpp"

namespace PMMA::Constants {
inline constexpr float PI = 3.141592653589793f;
inline constexpr float TAU = 6.283185307179586f;

inline constexpr int RENDER_PIPELINE_INSTANCE_MAX_SIZE = 16777216;
inline constexpr int MAX_TEXTURE_MIPS = 13; // Compressed, 12 mips, RGBA for generated, lookup, color

inline constexpr int MAX_FRAMES_BETWEEN_STALE_BUFFER_CLEANUP = 60; // 1 second at 60fps

struct Logging_Types {
    static inline constexpr std::string_view LOG_DEBUG = "[Debug]";
    static inline constexpr std::string_view LOG_INFO = "[Info]";
    static inline constexpr std::string_view LOG_WARN = "[Warn]";
    static inline constexpr std::string_view LOG_ERROR = "[Error]";
};

struct ANSI_Escape_Codes {
    // Reset
    static inline constexpr std::string_view RESET = "\033[0m";

    // Regular Colors
    static inline constexpr std::string_view BLACK = "\033[0;30m";
    static inline constexpr std::string_view RED = "\033[0;31m";
    static inline constexpr std::string_view GREEN = "\033[0;32m";
    static inline constexpr std::string_view YELLOW = "\033[0;33m";
    static inline constexpr std::string_view BLUE = "\033[0;34m";
    static inline constexpr std::string_view PURPLE = "\033[0;35m";
    static inline constexpr std::string_view CYAN = "\033[0;36m";
    static inline constexpr std::string_view WHITE = "\033[0;37m";

    // Bold Colors
    static inline constexpr std::string_view BOLD_BLACK = "\033[1;30m";
    static inline constexpr std::string_view BOLD_RED = "\033[1;31m";
    static inline constexpr std::string_view BOLD_GREEN = "\033[1;32m";
    static inline constexpr std::string_view BOLD_YELLOW = "\033[1;33m";
    static inline constexpr std::string_view BOLD_BLUE = "\033[1;34m";
    static inline constexpr std::string_view BOLD_PURPLE = "\033[1;35m";
    static inline constexpr std::string_view BOLD_CYAN = "\033[1;36m";
    static inline constexpr std::string_view BOLD_WHITE = "\033[1;37m";

    // Underline Colors
    static inline constexpr std::string_view UNDERLINE_BLACK = "\033[4;30m";
    static inline constexpr std::string_view UNDERLINE_RED = "\033[4;31m";
    static inline constexpr std::string_view UNDERLINE_GREEN = "\033[4;32m";
    static inline constexpr std::string_view UNDERLINE_YELLOW = "\033[4;33m";
    static inline constexpr std::string_view UNDERLINE_BLUE = "\033[4;34m";
    static inline constexpr std::string_view UNDERLINE_PURPLE = "\033[4;35m";
    static inline constexpr std::string_view UNDERLINE_CYAN = "\033[4;36m";
    static inline constexpr std::string_view UNDERLINE_WHITE = "\033[4;37m";

    // Background Colors
    static inline constexpr std::string_view BG_BLACK = "\033[40m";
    static inline constexpr std::string_view BG_RED = "\033[41m";
    static inline constexpr std::string_view BG_GREEN = "\033[42m";
    static inline constexpr std::string_view BG_YELLOW = "\033[43m";
    static inline constexpr std::string_view BG_BLUE = "\033[44m";
    static inline constexpr std::string_view BG_PURPLE = "\033[45m";
    static inline constexpr std::string_view BG_CYAN = "\033[46m";
    static inline constexpr std::string_view BG_WHITE = "\033[47m";

    // High Intensity Colors
    static inline constexpr std::string_view INTENSE_BLACK = "\033[0;90m";
    static inline constexpr std::string_view INTENSE_RED = "\033[0;91m";
    static inline constexpr std::string_view INTENSE_GREEN = "\033[0;92m";
    static inline constexpr std::string_view INTENSE_YELLOW = "\033[0;93m";
    static inline constexpr std::string_view INTENSE_BLUE = "\033[0;94m";
    static inline constexpr std::string_view INTENSE_PURPLE = "\033[0;95m";
    static inline constexpr std::string_view INTENSE_CYAN = "\033[0;96m";
    static inline constexpr std::string_view INTENSE_WHITE = "\033[0;97m";

    // Bold High Intensity Colors
    static inline constexpr std::string_view BOLD_INTENSE_BLACK = "\033[1;90m";
    static inline constexpr std::string_view BOLD_INTENSE_RED = "\033[1;91m";
    static inline constexpr std::string_view BOLD_INTENSE_GREEN = "\033[1;92m";
    static inline constexpr std::string_view BOLD_INTENSE_YELLOW = "\033[1;93m";
    static inline constexpr std::string_view BOLD_INTENSE_BLUE = "\033[1;94m";
    static inline constexpr std::string_view BOLD_INTENSE_PURPLE = "\033[1;95m";
    static inline constexpr std::string_view BOLD_INTENSE_CYAN = "\033[1;96m";
    static inline constexpr std::string_view BOLD_INTENSE_WHITE = "\033[1;97m";

    // High Intensity Backgrounds
    static inline constexpr std::string_view BG_INTENSE_BLACK = "\033[0;100m";
    static inline constexpr std::string_view BG_INTENSE_RED = "\033[0;101m";
    static inline constexpr std::string_view BG_INTENSE_GREEN = "\033[0;102m";
    static inline constexpr std::string_view BG_INTENSE_YELLOW = "\033[0;103m";
    static inline constexpr std::string_view BG_INTENSE_BLUE = "\033[0;104m";
    static inline constexpr std::string_view BG_INTENSE_PURPLE = "\033[0;105m";
    static inline constexpr std::string_view BG_INTENSE_CYAN = "\033[0;106m";
    static inline constexpr std::string_view BG_INTENSE_WHITE = "\033[0;107m";

    // Additional Styles
    static inline constexpr std::string_view STYLE_BOLD = "\033[1m";
    static inline constexpr std::string_view STYLE_ITALIC = "\033[3m";
    static inline constexpr std::string_view STYLE_UNDERLINE = "\033[4m";
    static inline constexpr std::string_view STYLE_STRIKETHROUGH = "\033[9m";
};

struct HatStates {
    static inline const std::string_view NOT_PRESSED = "HAT NOT PRESSED";
    static inline const std::string_view PRESSED_UP = "HAT PRESSED UP";
    static inline const std::string_view PRESSED_RIGHT = "HAT PRESSED RIGHT";
    static inline const std::string_view PRESSED_DOWN = "HAT PRESSED DOWN";
    static inline const std::string_view PRESSED_LEFT = "HAT PRESSED LEFT";
    static inline const std::string_view PRESSED_UP_RIGHT = "HAT PRESSED UP-RIGHT";
    static inline const std::string_view PRESSED_DOWN_RIGHT = "HAT PRESSED DOWN-RIGHT";
    static inline const std::string_view PRESSED_DOWN_LEFT = "HAT PRESSED DOWN-LEFT";
    static inline const std::string_view PRESSED_UP_LEFT = "HAT PRESSED UP-LEFT";
};

struct OperatingSystems {
    static inline const std::string_view ANDROID = "Android";
    static inline const std::string_view BSD = "BSD";
    static inline const std::string_view EMSCRIPTEN = "Emscripten";
    static inline const std::string_view HAIKU = "Haiku";
    static inline const std::string_view HURD = "Hurd";
    static inline const std::string_view IOS = "iOS";
    static inline const std::string_view LINUX = "Linux";
    static inline const std::string_view NX = "Nintendo Switch";
    static inline const std::string_view MACOS = "MacOS";
    static inline const std::string_view PS4 = "Play Station 4";
    static inline const std::string_view PS5 = "Play Station 5";
    static inline const std::string_view VISIONOS = "VisionOS";
    static inline const std::string_view WINDOWS = "Windows";
    static inline const std::string_view WINRT = "WinRT";
    static inline const std::string_view XBOXONE = "XboxOne";
    static inline const std::string_view UNKNOWN = "Unknown";
};

struct GraphicsBackends {
    static inline const std::string_view NO_RENDERER = "No Renderer";
    static inline const std::string_view DIRECT3D11 = "Direct3D 11.0";
    static inline const std::string_view DIRECT3D12 = "Direct3D 12.0";
    static inline const std::string_view GNM = "GNM"; // PlayStation (Developer license needed)
    static inline const std::string_view METAL = "Metal";
    static inline const std::string_view NVN = "NVN"; // Nintendo Switch
    static inline const std::string_view OPENGL_ES = "OpenGL ES";
    static inline const std::string_view OPENGL = "OpenGL";
    static inline const std::string_view VULKAN = "Vulkan";
    static inline const std::string_view UNKNOWN = "Unknown";
};

struct Colors {
    static inline constexpr std::string_view RED = "red";
    static inline constexpr std::string_view ORANGE = "ora";
    static inline constexpr std::string_view YELLOW = "yel";
    static inline constexpr std::string_view GREEN = "gre";
    static inline constexpr std::string_view BLUE = "blu";
    static inline constexpr std::string_view INDIGO = "ind";
    static inline constexpr std::string_view VIOLET = "vio";
    static inline constexpr std::string_view BLACK = "blk";
    static inline constexpr std::string_view WHITE = "wht";
    static inline constexpr std::string_view GRAY = "gry";
    static inline constexpr std::string_view CYAN = "cya";
    static inline constexpr std::string_view MAGENTA = "mag";
    static inline constexpr std::string_view LIGHT_RED = "lrd";
    static inline constexpr std::string_view LIGHT_ORANGE = "lor";
    static inline constexpr std::string_view LIGHT_YELLOW = "lyl";
    static inline constexpr std::string_view LIGHT_GREEN = "lgr";
    static inline constexpr std::string_view LIGHT_BLUE = "lbl";
    static inline constexpr std::string_view LIGHT_INDIGO = "lin";
    static inline constexpr std::string_view LIGHT_VIOLET = "lvi";
    static inline constexpr std::string_view SKY_BLUE = "sky";
    static inline constexpr std::string_view GOLD = "gol";
    static inline constexpr std::string_view SILVER = "slv";
    static inline constexpr std::string_view BROWN = "brn";
    static inline constexpr std::string_view PEA_GREEN = "pea";
    static inline constexpr std::string_view OLIVE = "olv";
    static inline constexpr std::string_view TAN = "tan";
    static inline constexpr std::string_view NAVY = "nav";
    static inline constexpr std::string_view MAROON = "mar";
    static inline constexpr std::string_view PURPLE = "pur";
    static inline constexpr std::string_view CORAL = "cor";
    static inline constexpr std::string_view TEAL = "tea";
    static inline constexpr std::string_view CHERRY = "che";
    static inline constexpr std::string_view LIME = "lim";
    static inline constexpr std::string_view MOCCASIN = "moc";
    static inline constexpr std::string_view BEIGE = "bei";
    static inline constexpr std::string_view DUSK = "dus";
    static inline constexpr std::string_view SALT = "slt";
    static inline constexpr std::string_view LAVENDER = "lav";
    static inline constexpr std::string_view PEACH = "pch";
    static inline constexpr std::string_view MINT = "mnt";
    static inline constexpr std::string_view ROSE = "rse";
    static inline constexpr std::string_view BRONZE = "brz";
    static inline constexpr std::string_view AQUAMARINE = "aqu";
    static inline constexpr std::string_view PERIWINKLE = "per";
    static inline constexpr std::string_view ICE_BLUE = "ice";
    static inline constexpr std::string_view PLUM = "plm";
    static inline constexpr std::string_view COPPER = "cop";
    static inline constexpr std::string_view CREAM = "crm";
    static inline constexpr std::string_view PINK = "pnk";
    static inline constexpr std::string_view FOREST = "for";
    static inline constexpr std::string_view SAND = "snd";
    static inline constexpr std::string_view AMBER = "amb";
    static inline constexpr std::string_view AZURE = "azr";
    static inline constexpr std::string_view TURQUOISE = "trq";
    static inline constexpr std::string_view COBALT = "cob";
    static inline constexpr std::string_view CHARCOAL = "chc";
    static inline constexpr std::string_view IVORY = "ivr";
    static inline constexpr std::string_view MUSCAT = "mus";
    static inline constexpr std::string_view OLIVE_DRAB = "old";
    static inline constexpr std::string_view SAGE_GREEN = "sgr";
    static inline constexpr std::string_view WHEAT = "whe";
    static inline constexpr std::string_view RUBY = "rub";
    static inline constexpr std::string_view EMERALD = "emr";
    static inline constexpr std::string_view SLIME_GREEN = "slm";
    static inline constexpr std::string_view ONYX = "ony";
    static inline constexpr std::string_view SPEARMINT = "spe";
    static inline constexpr std::string_view CHARTREUSE = "chr";
    static inline constexpr std::string_view BLOOD_RED = "bld";
    static inline constexpr std::string_view SPRING_GREEN = "spg";
    static inline constexpr std::string_view DARK_RED = "dre";
    static inline constexpr std::string_view DARK_ORANGE = "dor";
    static inline constexpr std::string_view DARK_YELLOW = "dye";
    static inline constexpr std::string_view DARK_GREEN = "dgr";
    static inline constexpr std::string_view DARK_BLUE = "dbl";
    static inline constexpr std::string_view DARK_INDIGO = "din";
    static inline constexpr std::string_view DARK_VIOLET = "dvi";
    static inline constexpr std::string_view DARK_GREY = "dgy";
    static inline constexpr std::string_view LIGHT_GREY = "lgy";
    static inline constexpr std::string_view OCHRE = "och";
    static inline constexpr std::string_view UMBER = "umb";
    static inline constexpr std::string_view TERRACOTTA = "ter";
    static inline constexpr std::string_view MUD_BROWN = "mud";
    static inline constexpr std::string_view SAPPHIRE = "sap";
    static inline constexpr std::string_view AMYTHYST = "amy";
    static inline constexpr std::string_view GARNET = "gnt";
    static inline constexpr std::string_view TAUPE = "tpe";
    static inline constexpr std::string_view BUBBLEGUM = "bub";
    static inline constexpr std::string_view MIST_ROSE = "mrs";
    static inline constexpr std::string_view HONEY = "hny";
    static inline constexpr std::string_view SEAFOAM = "sea";
    static inline constexpr std::string_view NEON_GREEN = "neo";
    static inline constexpr std::string_view ELECTRIC_PINK = "elc";
    static inline constexpr std::string_view SUNFLOWER = "sun";
    static inline constexpr std::string_view CRIMSON = "crl";
    static inline constexpr std::string_view CERULIAN = "cyl";
    static inline constexpr std::string_view MOSS_GREEN = "mgn";
    static inline constexpr std::string_view SAFFRON = "sfr";
    static inline constexpr std::string_view APRICOT = "apr";
    static inline constexpr std::string_view FLAX = "flx";
    static inline constexpr std::string_view MYSTIC_PURPLE = "mys";

    static constexpr std::array<PMMA::Internal::ColorEntry, 100> ColorMap = {{{RED, {255, 0, 0}},
                                                                              {ORANGE, {251, 79, 19}},
                                                                              {YELLOW, {255, 255, 0}},
                                                                              {GREEN, {0, 255, 0}},
                                                                              {BLUE, {0, 0, 255}},
                                                                              {INDIGO, {51, 0, 153}},
                                                                              {VIOLET, {143, 0, 255}},
                                                                              {BLACK, {0, 0, 0}},
                                                                              {WHITE, {255, 255, 255}},
                                                                              {GRAY, {128, 128, 128}},
                                                                              {CYAN, {0, 255, 255}},
                                                                              {MAGENTA, {255, 0, 255}},
                                                                              {LIGHT_RED, {255, 102, 102}},
                                                                              {LIGHT_ORANGE, {255, 178, 102}},
                                                                              {LIGHT_YELLOW, {255, 255, 153}},
                                                                              {LIGHT_GREEN, {153, 255, 153}},
                                                                              {LIGHT_BLUE, {153, 204, 255}},
                                                                              {LIGHT_INDIGO, {109, 90, 207}},
                                                                              {LIGHT_VIOLET, {204, 153, 255}},
                                                                              {SKY_BLUE, {135, 206, 235}},
                                                                              {GOLD, {255, 215, 0}},
                                                                              {SILVER, {192, 192, 192}},
                                                                              {BROWN, {150, 75, 0}},
                                                                              {PEA_GREEN, {142, 209, 63}},
                                                                              {OLIVE, {128, 128, 0}},
                                                                              {TAN, {210, 180, 140}},
                                                                              {NAVY, {0, 0, 128}},
                                                                              {MAROON, {128, 0, 0}},
                                                                              {PURPLE, {128, 0, 128}},
                                                                              {CORAL, {255, 127, 80}},
                                                                              {TEAL, {0, 128, 128}},
                                                                              {CHERRY, {255, 20, 147}},
                                                                              {LIME, {204, 255, 153}},
                                                                              {MOCCASIN, {255, 228, 181}},
                                                                              {BEIGE, {245, 245, 220}},
                                                                              {DUSK, {169, 169, 169}},
                                                                              {SALT, {211, 211, 211}},
                                                                              {LAVENDER, {230, 230, 250}},
                                                                              {PEACH, {255, 218, 185}},
                                                                              {MINT, {48, 128, 20}},
                                                                              {ROSE, {255, 0, 127}},
                                                                              {BRONZE, {205, 127, 50}},
                                                                              {AQUAMARINE, {127, 255, 212}},
                                                                              {PERIWINKLE, {218, 112, 214}},
                                                                              {ICE_BLUE, {240, 255, 255}},
                                                                              {PLUM, {221, 160, 221}},
                                                                              {COPPER, {184, 115, 51}},
                                                                              {CRIMSON, {255, 253, 208}},
                                                                              {PINK, {255, 192, 203}},
                                                                              {FOREST, {34, 139, 34}},
                                                                              {SAND, {194, 178, 128}},
                                                                              {AMBER, {255, 191, 0}},
                                                                              {AZURE, {0, 127, 255}},
                                                                              {TURQUOISE, {64, 224, 208}},
                                                                              {COBALT, {0, 71, 71}},
                                                                              {CHARCOAL, {54, 69, 79}},
                                                                              {IVORY, {255, 255, 240}},
                                                                              {MUSCAT, {255, 250, 205}},
                                                                              {OLIVE_DRAB, {107, 142, 35}},
                                                                              {SAGE_GREEN, {189, 183, 107}},
                                                                              {WHEAT, {245, 222, 179}},
                                                                              {RUBY, {224, 17, 95}},
                                                                              {EMERALD, {80, 200, 120}},
                                                                              {SLIME_GREEN, {192, 255, 62}},
                                                                              {ONYX, {255, 8, 127}},
                                                                              {SPEARMINT, {255, 239, 213}},
                                                                              {CHARTREUSE, {127, 255, 0}},
                                                                              {BLOOD_RED, {139, 0, 0}},
                                                                              {SPRING_GREEN, {154, 205, 50}},
                                                                              {DARK_RED, {138, 0, 0}},
                                                                              {DARK_ORANGE, {255, 140, 0}},
                                                                              {DARK_YELLOW, {204, 204, 0}},
                                                                              {DARK_GREEN, {0, 100, 0}},
                                                                              {DARK_BLUE, {0, 0, 139}},
                                                                              {DARK_INDIGO, {75, 0, 130}},
                                                                              {DARK_VIOLET, {148, 0, 211}},
                                                                              {DARK_GREY, {64, 64, 64}},
                                                                              {LIGHT_GREY, {218, 218, 218}},
                                                                              {OCHRE, {204, 119, 34}},
                                                                              {UMBER, {99, 81, 71}},
                                                                              {TERRACOTTA, {182, 106, 80}},
                                                                              {MUD_BROWN, {96, 70, 15}},
                                                                              {SAPPHIRE, {15, 82, 186}},
                                                                              {AMYTHYST, {153, 102, 204}},
                                                                              {GARNET, {173, 54, 79}},
                                                                              {TAUPE, {72, 60, 50}},
                                                                              {BUBBLEGUM, {255, 182, 193}},
                                                                              {MIST_ROSE, {255, 228, 255}},
                                                                              {HONEY, {255, 183, 76}},
                                                                              {SEAFOAM, {178, 255, 255}},
                                                                              {NEON_GREEN, {57, 255, 20}},
                                                                              {ELECTRIC_PINK, {255, 0, 102}},
                                                                              {SUNFLOWER, {255, 204, 0}},
                                                                              {CRIMSON, {255, 64, 64}},
                                                                              {CERULIAN, {0, 191, 255}},
                                                                              {MOSS_GREEN, {46, 139, 87}},
                                                                              {SAFFRON, {255, 99, 71}},
                                                                              {APRICOT, {255, 165, 79}},
                                                                              {FLAX, {238, 232, 205}},
                                                                              {MYSTIC_PURPLE, {102, 51, 153}}}};
};
} // namespace PMMA::Constants