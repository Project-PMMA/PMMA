#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>

#include "Internal/Core./PMMA_Registry.hpp"
#include "Internal/SavedDataManager.hpp"

namespace {
struct FileData {
    PMMA::Internal::VersionData LatestVersion;
    std::string CheckAgain;
};

nlohmann::json VersionDataToJson(
    const PMMA::Internal::VersionData &version) {
    nlohmann::json json;

    json["version"] = version.Version;

    json["version_code"]["major"] =
        version.VersionCodes[0];

    json["version_code"]["minor"] =
        version.VersionCodes[1];

    json["version_code"]["patch"] =
        version.VersionCodes[2];

    json["pre_release"] =
        version.PreRelease;

    return json;
}

PMMA::Internal::VersionData VersionDataFromJson(
    const nlohmann::json &json) {
    PMMA::Internal::VersionData version{};

    version.Version =
        json.value("version", "");

    if (json.contains("version_code")) {
        const auto &versionCode =
            json["version_code"];

        version.VersionCodes[0] =
            versionCode.value(
                "major",
                uint16_t{0});

        version.VersionCodes[1] =
            versionCode.value(
                "minor",
                uint16_t{0});

        version.VersionCodes[2] =
            versionCode.value(
                "patch",
                uint16_t{0});
    } else {
        version.VersionCodes[0] = 0;
        version.VersionCodes[1] = 0;
        version.VersionCodes[2] = 0;
    }

    version.PreRelease =
        json.value("pre_release", false);

    return version;
}

nlohmann::json FileDataToJson(
    const FileData &data) {
    nlohmann::json json;

    json["latest_version"] =
        VersionDataToJson(data.LatestVersion);

    json["check_again"] =
        data.CheckAgain;

    return json;
}

FileData FileDataFromJson(
    const nlohmann::json &json) {
    FileData data{};

    if (json.contains("latest_version") &&
        json["latest_version"].is_object()) {
        data.LatestVersion =
            VersionDataFromJson(
                json["latest_version"]);
    }

    data.CheckAgain =
        json.value("check_again", "");

    return data;
}
} // namespace

void PMMA::Internal::SavedDataManager::Load() {
    const std::string filepath =
        PMMA::Core::Registry::PMMA_Location +
        PMMA::Core::Registry::PathSeparator +
        "config" +
        PMMA::Core::Registry::PathSeparator +
        "config.json";

    std::ifstream file(filepath);

    if (!file)
        return;

    try {
        nlohmann::json json;

        file >> json;

        if (json.empty())
            return;

        const FileData data =
            FileDataFromJson(json);

        PMMA::Core::Registry::Latest_PMMA_Version =
            data.LatestVersion;

        CheckAgain = data.CheckAgain;
    } catch (const nlohmann::json::exception &) {
        return;
    }
}

void PMMA::Internal::SavedDataManager::Save() {
    FileData data;

    data.LatestVersion =
        PMMA::Core::Registry::Latest_PMMA_Version;

    data.CheckAgain = PMMA::Core::Registry::UpdateCheckTime;

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

    data.CheckAgain =
        date.str();

    const nlohmann::json json =
        FileDataToJson(data);

    const std::string filepath =
        PMMA::Core::Registry::PMMA_Location +
        PMMA::Core::Registry::PathSeparator +
        "config" +
        PMMA::Core::Registry::PathSeparator +
        "config.json";

    std::ofstream file(filepath);

    if (!file)
        return;

    file << json.dump(4);
}
