#include <numeric>
#include <string>
#include <thread>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"
#include "Internal/LoggingManager.hpp"
#include "Internal/NetworkingManager.hpp"
#include "Internal/ParallelWorker.hpp"
#include "Internal/PowerSavingManager.hpp"
#include "Internal/SavedDataManager.hpp"
#include "Internal/TerminalColorChecker.hpp"

#include "PMMA.hpp"

namespace PMMA {
void Initialize(std::string location) {
    if (std::filesystem::exists(location)) {
        if (!std::filesystem::is_directory(location)) {
            std::cerr << "The provided PMMA location is not a directory: " << location << std::endl;
            throw std::runtime_error("The provided PMMA location is not a directory: " + location);
        }
    } else {
        std::cerr << "The provided PMMA location does not exist: " << location << std::endl;
        throw std::runtime_error("The provided PMMA location does not exist: " + location);
    }

    PMMA::Internal::TerminalColorChecker TerminalColorCheckerInstance;

    PMMA::Core::Registry::PMMA_Location = location;

    PMMA::Core::Registry::RandomSeedGenerator.seed(std::random_device{}());

    PMMA::Core::ParallelWorkerInstance = new PMMA::Internal::ParallelWorker(PMMA::Core::Registry::ParallelWorkerMaxThreads);

    PMMA::Core::LoggingManagerInstance = new PMMA::Internal::LoggingManager();

    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        0,
        "PMMA logging initialized, log files are named: 'DD-MM-YYYY at HH-MM-SS.txt'.");

    if (PMMA::Core::Registry::Current_PMMA_Version.PreRelease) {
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            12,
            "Welcome to Python Multi-Media API (PMMA) version: " + PMMA::Core::Registry::Current_PMMA_Version.Version + " (pre-release).");
    } else {
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            12,
            "Welcome to Python Multi-Media API (PMMA) version: " + PMMA::Core::Registry::Current_PMMA_Version.Version + ".");
    }

    std::string OperatingSystem = PMMA::General::GetOperatingSystem();
    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        46,
        "You are running on the Operating System: '" + OperatingSystem + "'.");

    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        14,
        "Please note that PMMA is currently in a developmental state, \
meaning that the API is subject to change - we are hoping to remove this \
warning and improve backwards compatibility in PMMA 6.");

    PMMA::Core::Registry::IsPowerSavingModeEnabled = PMMA::General::Is_Power_Saving_Mode_Enabled(true);

    if (PMMA::Core::Registry::CPU_Supports_AVX512) {
        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            3,
            "PMMA has detected that your system has AVX-512 support \
and will automatically use it where applicable. AVX-512 allows for up to \
16 operations to be performed simultaneously on the CPU.");
    } else {
        if (PMMA::Core::Registry::CPU_Supports_AVX2) {
            PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                4,
                "PMMA has detected that your system has AVX2 support \
and will automatically use it where applicable. AVX2 allows for up to \
8 operations to be performed simultaneously on the CPU.");
        } else {
            PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                5,
                "PMMA has detected that your system does not have any \
support for AVX-512 or AVX2. This will not affect the usability of PMMA \
but may result in reduced performance.");
        }
    }

#ifdef USE_PYTHON
    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        6,
        "PMMA has been built with compatibility for the Python programming language!");
#else
    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        6,
        "PMMA has not been built with additional compatibility \
for Python, this does not effect the operation of PMMA but will change \
how PMMA and Python interact.");
#endif

    PMMA::Core::PowerSavingManagerInstance->PowerSavingModeCheckingThread = std::thread(
        &PMMA::Internal::PowerSavingManager::PowerSavingUpdaterThread,
        PMMA::Core::PowerSavingManagerInstance);

    PMMA::Core::Registry::SecondaryDisplayIDs.reserve(255);
    PMMA::Core::Registry::SecondaryDisplayIDs.resize(255);
    std::iota(
        PMMA::Core::Registry::SecondaryDisplayIDs.begin(),
        PMMA::Core::Registry::SecondaryDisplayIDs.end(), 1);

    PMMA::Core::NetworkingManagerInstance = new PMMA::Internal::NetworkingManager();
    PMMA::Core::SavedDataManagerInstance = new PMMA::Internal::SavedDataManager();

    // NOTE: DATA LOADING IS IN PARALLEL AS ONLY USED FOR UPDATE CHECKING FOR NOW

    PMMA::Core::ParallelWorkerInstance->Enqueue([]() {
        PMMA::Core::SavedDataManagerInstance->Load();

        if (PMMA::Core::SavedDataManagerInstance->ShouldCheckForUpdates()) {
            PMMA::Core::NetworkingManagerInstance->QueryLatest_PMMA_Version();
        }

        if (PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[0] > PMMA::Core::Registry::Current_PMMA_Version.VersionCodes[0]) {
            PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                77,
                "A new major version of PMMA is available: " + PMMA::Core::Registry::Latest_PMMA_Version.Version + ". \
You are currently using version: " +
                    PMMA::Core::Registry::Current_PMMA_Version.Version + ".");

        } else if (PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[1] > PMMA::Core::Registry::Current_PMMA_Version.VersionCodes[1]) {
            PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                78,
                "A new minor version of PMMA is available: " + PMMA::Core::Registry::Latest_PMMA_Version.Version + ". \
You are currently using version: " +
                    PMMA::Core::Registry::Current_PMMA_Version.Version + ".");
        } else if (PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[2] > PMMA::Core::Registry::Current_PMMA_Version.VersionCodes[2]) {
            PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                79,
                "A new bug fixed version of PMMA is available: " + PMMA::Core::Registry::Latest_PMMA_Version.Version + ". \
You are currently using version: " +
                    PMMA::Core::Registry::Current_PMMA_Version.Version + ".");
        } else {
            PMMA::Core::LoggingManagerInstance->InternalLogInfo(
                17,
                "You are on the latest version of PMMA!");
        }

        bool UnreleasedVersion = ((PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[0] < PMMA::Core::Registry::Current_PMMA_Version.VersionCodes[0]) ||
                                  (PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[1] < PMMA::Core::Registry::Current_PMMA_Version.VersionCodes[1]) ||
                                  (PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[2] < PMMA::Core::Registry::Current_PMMA_Version.VersionCodes[2]));

        if (UnreleasedVersion) {
            PMMA::Core::LoggingManagerInstance->InternalLogDebug(
                22,
                "Thank you for using a pre-released version of PMMA! Please \
note that there will likely be issues or missing/broken features as we work \
towards creating the next version of the API. If you find any bugs or think \
something could be improved it would be invaluable for you to let us know \
by creating a new issue here: 'https://github.com/Project-PMMA/PMMA/issues'.");
        }

        PMMA::Core::SavedDataManagerInstance->Save();
    });
}

void Uninitialize() {
    PMMA::Core::PowerSavingManagerInstance->stop();

    delete PMMA::Core::LoggingManagerInstance;
    PMMA::Core::LoggingManagerInstance = nullptr;

    delete PMMA::Core::PowerSavingManagerInstance;
    PMMA::Core::PowerSavingManagerInstance = nullptr;

    delete PMMA::Core::NetworkingManagerInstance;
    PMMA::Core::NetworkingManagerInstance = nullptr;
}
} // namespace PMMA