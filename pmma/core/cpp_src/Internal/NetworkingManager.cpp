#include <JSON/json.hpp>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"

#include "Internal/LoggingManager.hpp"
#include "Internal/NetworkingManager.hpp"

void PMMA::Internal::NetworkingManager::QueryLatest_PMMA_Version() {
    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        74,
        "Querying GitHub for the latest PMMA version...");

    PMMA::Core::LoggingManagerInstance->InternalLogDebug(
        75, "PMMA is connecting to the internet to check for new versions of PMMA. \
PMMA is not using this to phone home, collect any usage data or send any information \
about you or your computer to any third party. You are welcome to check the source \
code of PMMA to verify this if you wish. Note however, the server PMMA is connecting \
to is a third party server (GitHub) and PMMA has no control over what data GitHub collects.");

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
        std::cerr << "GitHub request failed: "
                  << r.status_code << '\n';
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

        return;
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "Failed to parse GitHub response: "
                  << e.what() << '\n';
        return;
    }
}