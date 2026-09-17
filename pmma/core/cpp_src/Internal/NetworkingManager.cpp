#include <iostream>
#include <string>

#include <JSON/json.hpp>
#include <cpr/cpr.h>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"

#include "Internal/LoggingManager.hpp"
#include "Internal/NetworkingManager.hpp"

void PMMA::Internal::NetworkingManager::QueryLatest_PMMA_Version() {
    if (!PMMA::Core::LoggingManagerInstance->InternalLogDebug(
            75, "PMMA is connecting to the internet to check for new versions of PMMA. \
PMMA is not using this to phone home, collect any usage data or send any information \
about you or your computer to any third party. You are welcome to check the source \
code of PMMA to verify this if you wish. Note however, the server PMMA is connecting \
to is a third party server (GitHub) and PMMA has no control over what data GitHub collects.")) {

        PMMA::Core::LoggingManagerInstance->InternalLogInfo(
            74,
            "Querying GitHub for the latest PMMA version...");
    }

    std::string URL;

    if (PMMA::Core::Registry::Current_PMMA_Version.PreRelease) {
        URL = "https://project-pmma.github.io/PMMA-Website/api/pre-release.json";
    } else {
        URL = "https://project-pmma.github.io/PMMA-Website/api/release.json";
    }

    cpr::Response r = cpr::Get(
        cpr::Url{URL},
        cpr::Header{
            {"User-Agent", "PMMA/" + PMMA::Core::Registry::Current_PMMA_Version.Version},
            {"Accept", "application/vnd.github+json"}});

    if (r.status_code != 200) {
        PMMA::Core::LoggingManagerInstance->InternalLogWarn(
            76,
            "PMMA was unable to check for the latest version of PMMA on GitHub. Error code: " + std::to_string(r.status_code));
        return;
    }

    try {
        nlohmann::json response = nlohmann::json::parse(r.text);

        if (response.empty()) {
            return;
        }

        auto version_code = response["version_code"];
        uint16_t major = version_code["major"].get<uint16_t>();
        uint16_t minor = version_code["minor"].get<uint16_t>();
        uint16_t patch = version_code["patch"].get<uint16_t>();

        PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[0] = major;
        PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[1] = minor;
        PMMA::Core::Registry::Latest_PMMA_Version.VersionCodes[2] = patch;

        PMMA::Core::Registry::Latest_PMMA_Version.PreRelease = response["pre release"].get<bool>();

        PMMA::Core::Registry::Latest_PMMA_Version.Version =
            std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

        const auto now =
            std::chrono::system_clock::now();

        const auto checkAgain =
            now + std::chrono::hours(24 * 7);

        const std::time_t checkAgainTime =
            std::chrono::system_clock::to_time_t(
                checkAgain);

        std::tm checkAgainTm{};

#ifdef _WIN32

        localtime_s(
            &checkAgainTm,
            &checkAgainTime);

#else

        localtime_r(
            &checkAgainTime,
            &checkAgainTm);

#endif

        std::ostringstream date;

        date << std::put_time(
            &checkAgainTm,
            "%Y-%m-%d");

        PMMA::Core::Registry::UpdateCheckTime =
            date.str();

        return;
    } catch (const nlohmann::json::exception &e) {
        PMMA::Core::LoggingManagerInstance->InternalLogWarn(
            80,
            "PMMA was unable to understand the new version data from the server." + std::string(e.what()));

        return;
    }
}