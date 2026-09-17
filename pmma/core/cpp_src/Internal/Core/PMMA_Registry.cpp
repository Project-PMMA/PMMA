#include "Internal/Utility/CPU_FeatureSetUtils.hpp"

#include "Internal/Core/PMMA_Registry.hpp"

#include <filesystem>

namespace PMMA::Core::Registry {
std::vector<unsigned char> SecondaryDisplayIDs;

std::string PMMA_Location = "";
std::string PathSeparator = std::string(1, std::filesystem::path::preferred_separator);
PMMA::Internal::VersionData Current_PMMA_Version = {
    "5.1.0",
    {5, 1, 0},
    true};
PMMA::Internal::VersionData Latest_PMMA_Version;
std::string Locale = "en-US";
std::string UpdateCheckTime;

std::mutex SeedGeneratorLock;
std::mt19937 RandomSeedGenerator;
std::uniform_int_distribution<uint32_t> SeedDistribution;

std::chrono::steady_clock::time_point StartupTime = std::chrono::high_resolution_clock::now();
std::optional<std::chrono::steady_clock::time_point> TextureCompilationStartTime;

unsigned int KeyboardEventInstanceCount = 0;
unsigned int TextEventInstanceCount = 0;
unsigned int MousePositionEventInstanceCount = 0;
unsigned int MouseEnterWindowEventInstanceCount = 0;
unsigned int MouseButtonEventInstanceCount = 0;
unsigned int MouseScrollEventInstanceCount = 0;
unsigned int ControllerEventInstanceCount = 0;
unsigned int DropEventInstanceCount = 0;

unsigned int RollingViewID;
unsigned int MaxViewID = 0;

unsigned int ParallelWorkerMaxThreads = std::thread::hardware_concurrency() - 1;

bool CPU_Supports_AVX2 = PMMA::Utility::CPU_FeatureSet::SupportsAVX2();
bool CPU_Supports_AVX512 = PMMA::Utility::CPU_FeatureSet::SupportsAVX512();
bool IsPowerSavingModeEnabled = false;
bool IsDebuggingModeEnabled = true;
bool IsApplicationRunning = true;
bool EscapeKeyShouldCloseWindow = false;
bool UserSetEscapeKeyShouldCloseWindow = false;
bool F11KeyShouldToggleFullScreen = true;
bool InitialSetup = true;
bool Is_PMMA_VersionReleased = true;
bool TerminalSupportsColor = false;
} // namespace PMMA::Core::Registry