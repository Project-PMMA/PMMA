#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <filesystem>
#include <fstream>
#endif

#include <bgfx/bgfx.h>
#include <bx/platform.h>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"

#include "Internal/LoggingManager.hpp"
#include "Internal/PowerSavingManager.hpp"

#include "Constants.hpp"
#include "Display.hpp"
#include "General.hpp"

std::string PMMA::General::Get_PMMA_Location() {
    return PMMA::Core::Registry::PMMA_Location;
}

bool PMMA::General::Is_Power_Saving_Mode_Enabled(bool ForceRefresh) {
    if (!ForceRefresh) {
        return PMMA::Core::Registry::IsPowerSavingModeEnabled; // Return cached value if not forcing a refresh
    }

#if defined(_WIN32)
    SYSTEM_POWER_STATUS power_status = {};
    if (GetSystemPowerStatus(&power_status)) {
        if (power_status.SystemStatusFlag == 1) {
            if (!PMMA::Core::Registry::IsPowerSavingModeEnabled) {
                PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                    1,
                    "Your device is running in power saving mode.", true);
            }
            PMMA::Core::Registry::IsPowerSavingModeEnabled = true;
            PMMA::Core::PowerSavingManagerInstance->updateCounter = 30;
            return true;
        }
        if (power_status.ACLineStatus == 0 && power_status.BatteryLifePercent <= 20) {
            if (!PMMA::Core::Registry::IsPowerSavingModeEnabled) {
                PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                    1,
                    "Your device is running in power saving mode.", true);
            }
            PMMA::Core::Registry::IsPowerSavingModeEnabled = true;
            PMMA::Core::PowerSavingManagerInstance->updateCounter = 30;
            return true; // Low battery test
        }
    }

    if (PMMA::Core::Registry::IsPowerSavingModeEnabled) {
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            2,
            "Your device is not running in power saving mode.", true);
    }
    PMMA::Core::Registry::IsPowerSavingModeEnabled = false;
    PMMA::Core::PowerSavingManagerInstance->updateCounter = 15;
    return false;

#elif defined(__linux__)
    const std::string powerPath = "/sys/class/power_supply/";

    try {
        for (const auto &entry : std::filesystem::directory_iterator(powerPath)) {
            if (entry.is_directory() && entry.path().filename().string().find("BAT") == 0) {
                std::ifstream statusFile(entry.path() / "status");
                std::string status;
                if (statusFile >> status && status == "Discharging") {
                    if (!PMMA::Core::Registry::IsPowerSavingModeEnabled) {
                        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                            1,
                            "Your device is running in power saving mode.", true);
                    }
                    PMMA::Core::Registry::IsPowerSavingModeEnabled = true;
                    PMMA::Core::PowerSavingManagerInstance->updateCounter = 30;
                    return true;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &error) {
        std::cerr << "Filesystem error: " << error.what() << "\n";
    }
    if (PMMA::Core::Registry::IsPowerSavingModeEnabled) {
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            2,
            "Your device is not running in power saving mode.", true);
    }
    PMMA::Core::Registry::IsPowerSavingModeEnabled = false;
    PMMA::Core::PowerSavingManagerInstance->updateCounter = 15;
    return false;

#else
    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        7,
        "Your platform is not supported for power saving mode \
checking using PMMA.");

    if (PMMA::Core::Registry::IsPowerSavingModeEnabled) {
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            2,
            "Your device is not running in power saving mode.", true);
    }
    PMMA::Core::Registry::IsPowerSavingModeEnabled = false;
    PMMA::Core::PowerSavingManagerInstance->running = false;
    PMMA::Core::PowerSavingManagerInstance->updateCounter = 5;
    return false;
#endif
}

bool PMMA::General::Is_DebugModeEnabled() {
    return PMMA::Core::Registry::IsDebuggingModeEnabled;
}

void PMMA::General::Set_DebugModeEnabled(bool DebugMode) {
    PMMA::Core::Registry::IsDebuggingModeEnabled = DebugMode;
}

bool PMMA::General::IsWindowCreated() {
    return (PMMA::Core::ActiveDisplayInstance != nullptr && PMMA::Core::ActiveDisplayInstance->GetIsWindowCreated());
}

bool PMMA::General::IsApplicationRunning() {
    return PMMA::Core::Registry::IsApplicationRunning;
}

bool PMMA::General::IsEscapeKeyToCloseWindow() {
    return PMMA::Core::Registry::EscapeKeyShouldCloseWindow;
}

void PMMA::General::SetEscapeKeyToCloseWindow(bool EscapeKeyShouldCloseWindow) {
    PMMA::Core::Registry::EscapeKeyShouldCloseWindow = EscapeKeyShouldCloseWindow;
    PMMA::Core::Registry::UserSetEscapeKeyShouldCloseWindow = true;
}

bool PMMA::General::IsF11KeyToToggleFullscreen() {
    return PMMA::Core::Registry::F11KeyShouldToggleFullScreen;
}

void PMMA::General::SetF11KeyToToggleFullscreen(bool F11KeyShouldToggleFullScreen) {
    PMMA::Core::Registry::F11KeyShouldToggleFullScreen = F11KeyShouldToggleFullScreen;
}

std::string PMMA::General::GetCurrent_PMMA_Version() {
    return PMMA::Core::Registry::Current_PMMA_Version.Version;
}

std::string PMMA::General::GetLatest_PMMA_Version() {
    return PMMA::Core::Registry::Latest_PMMA_Version.Version;
}

/*bool PMMA::General::IsUpdateAvailable() {
    std::string padded_current_version;
    std::string split_current_version[3];
    unsigned int split_count = 0;
    for (unsigned int i = 0; i < PMMA::Core::Registry::Current_PMMA_Version.length(); i++) {
        if (PMMA::Core::Registry::Current_PMMA_Version[i] == '.') {
            split_count++;
            continue;
        }
        split_current_version[split_count] += PMMA::Core::Registry::Current_PMMA_Version[i];
    }

    for (unsigned int i = 0; i < 3; i++) {
        padded_current_version += PadVersionString(split_current_version[i]);
    }

    std::string padded_latest_version;
    std::string split_latest_version[3];
    split_count = 0;
    for (unsigned int i = 0; i < PMMA::Core::Registry::Latest_PMMA_Version.length(); i++) {
        if (PMMA::Core::Registry::Latest_PMMA_Version[i] == '.') {
            split_count++;
            continue;
        }
        split_latest_version[split_count] += PMMA::Core::Registry::Latest_PMMA_Version[i];
    }

    for (unsigned int i = 0; i < 3; i++) {
        padded_latest_version += PadVersionString(split_latest_version[i]);
    }

    uint64_t numerical_current_version = std::stoull(padded_current_version);
    uint64_t numerical_latest_version = std::stoull(padded_latest_version);

    if (numerical_current_version > numerical_latest_version) {
        PMMA::Core::LoggingManagerInstance->InternalLogDebug(
            22,
            "Thank you for using a pre-released version of PMMA! Please \
note that there will likely be issues or missing/broken features as we work \
towards creating the next version of the API. If you find any bugs or think \
something could be improved it would be invaluable for you to let us know \
by creating a new issue here: 'https://github.com/Project-PMMA/PMMA/issues'.");
    }

    return numerical_current_version < numerical_latest_version;
}*/

double PMMA::General::GetApplicationStartTime() {
    return PMMA::Core::Registry::StartupTime.time_since_epoch().count() / 1000000000.0;
}

double PMMA::General::GetApplicationRunTime() {
    std::chrono::high_resolution_clock::time_point current_time = std::chrono::high_resolution_clock::now();
    return (current_time - PMMA::Core::Registry::StartupTime).count() / 1000000000.0;
}

void PMMA::General::SetLocale(std::string locale) {
    PMMA::Core::Registry::Locale = locale;
}

std::string PMMA::General::GetLocale() {
    return PMMA::Core::Registry::Locale;
}

std::string PMMA::General::GetOperatingSystem() {
#if BX_PLATFORM_ANDROID
    return std::string(PMMA::Constants::OperatingSystems::ANDROID);
#elif BX_PLATFORM_BSD
    return std::string(PMMA::Constants::OperatingSystems::BSD);
#elif BX_PLATFORM_EMSCRIPTEN
    return std::string(PMMA::Constants::OperatingSystems::EMSCRIPTEN);
#elif BX_PLATFORM_HAIKU
    return std::string(PMMA::Constants::OperatingSystems::HAIKU);
#elif BX_PLATFORM_HURD
    return std::string(PMMA::Constants::OperatingSystems::HURD);
#elif BX_PLATFORM_IOS
    return std::string(PMMA::Constants::OperatingSystems::IOS);
#elif BX_PLATFORM_LINUX
    return std::string(PMMA::Constants::OperatingSystems::LINUX);
#elif BX_PLATFORM_NX
    return std::string(PMMA::Constants::OperatingSystems::NX);
#elif BX_PLATFORM_OSX
    return std::string(PMMA::Constants::OperatingSystems::OSX);
#elif BX_PLATFORM_PS4
    return std::string(PMMA::Constants::OperatingSystems::PS4);
#elif BX_PLATFORM_PS5
    return std::string(PMMA::Constants::OperatingSystems::PS5);
#elif BX_PLATFORM_VISIONOS
    return std::string(PMMA::Constants::OperatingSystems::VISIONOS);
#elif BX_PLATFORM_WINDOWS
    return std::string(PMMA::Constants::OperatingSystems::WINDOWS);
#elif BX_PLATFORM_WINRT
    return std::string(PMMA::Constants::OperatingSystems::WINRT);
#elif BX_PLATFORM_XBOXONE
    return std::string(PMMA::Constants::OperatingSystems::XBOXONE);
#else
    return std::string(PMMA::Constants::OperatingSystems::UNKNOWN);
#endif
}

std::string PMMA::General::GetGraphicsBackend() {
    bgfx::RendererType::Enum backend = bgfx::getRendererType();

    switch (backend) {
    case bgfx::RendererType::Noop:
        return std::string(PMMA::Constants::GraphicsBackends::NO_RENDERER);
    case bgfx::RendererType::Direct3D11:
        return std::string(PMMA::Constants::GraphicsBackends::DIRECT3D11);
    case bgfx::RendererType::Direct3D12:
        return std::string(PMMA::Constants::GraphicsBackends::DIRECT3D12);
    case bgfx::RendererType::Gnm:
        return std::string(PMMA::Constants::GraphicsBackends::GNM);
    case bgfx::RendererType::Metal:
        return std::string(PMMA::Constants::GraphicsBackends::METAL);
    case bgfx::RendererType::Nvn:
        return std::string(PMMA::Constants::GraphicsBackends::NVN);
    case bgfx::RendererType::OpenGLES:
        return std::string(PMMA::Constants::GraphicsBackends::OPENGL_ES);
    case bgfx::RendererType::OpenGL:
        return std::string(PMMA::Constants::GraphicsBackends::OPENGL);
    case bgfx::RendererType::Vulkan:
        return std::string(PMMA::Constants::GraphicsBackends::VULKAN);
    default:
        return std::string(PMMA::Constants::GraphicsBackends::UNKNOWN);
    }
}

void PMMA::General::SetMaxParallelWorkerThreads(unsigned int max_threads) {
    PMMA::Core::Registry::ParallelWorkerMaxThreads = max_threads;
}

unsigned int PMMA::General::GetMaxParallelWorkerThreads() {
    return PMMA::Core::Registry::ParallelWorkerMaxThreads;
}

bool PMMA::General::IsCPU_Supports_AVX2() {
    return PMMA::Core::Registry::CPU_Supports_AVX2;
}

bool PMMA::General::IsCPU_Supports_AVX512() {
    return PMMA::Core::Registry::CPU_Supports_AVX512;
}

bool PMMA::General::Is_PMMA_VersionReleased() {
    return PMMA::Core::Registry::Is_PMMA_VersionReleased;
}