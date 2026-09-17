#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <random>
#include <string>
#include <vector>

namespace PMMA::Internal {
struct VersionData {
    std::string Version;
    uint16_t VersionCodes[3]; // major, minor, micro
    bool PreRelease;
};
} // namespace PMMA::Internal

namespace PMMA::Core::Registry {
extern std::vector<unsigned char> SecondaryDisplayIDs;
extern std::string PMMA_Location;
extern std::string PathSeparator;
extern PMMA::Internal::VersionData Current_PMMA_Version;
extern PMMA::Internal::VersionData Latest_PMMA_Version;
extern std::string Locale;

extern std::mutex SeedGeneratorLock;
extern std::mt19937 RandomSeedGenerator;
extern std::uniform_int_distribution<uint32_t> SeedDistribution;

extern std::chrono::high_resolution_clock::time_point StartupTime;
extern std::optional<std::chrono::steady_clock::time_point> TextureCompilationStartTime;

extern unsigned int KeyboardEventInstanceCount;
extern unsigned int TextEventInstanceCount;
extern unsigned int MousePositionEventInstanceCount;
extern unsigned int MouseEnterWindowEventInstanceCount;
extern unsigned int MouseButtonEventInstanceCount;
extern unsigned int MouseScrollEventInstanceCount;
extern unsigned int ControllerEventInstanceCount;
extern unsigned int DropEventInstanceCount;

extern unsigned int RollingViewID;
extern unsigned int MaxViewID;

extern unsigned int ParallelWorkerMaxThreads;

extern bool CPU_Supports_AVX2;
extern bool CPU_Supports_AVX512;
extern bool IsPowerSavingModeEnabled;
extern bool IsDebuggingModeEnabled;
extern bool IsApplicationRunning;
extern bool EscapeKeyShouldCloseWindow;
extern bool UserSetEscapeKeyShouldCloseWindow;
extern bool F11KeyShouldToggleFullScreen;
extern bool InitialSetup;
extern bool Is_PMMA_VersionReleased;
extern bool TerminalSupportsColor;
} // namespace PMMA::Core::Registry