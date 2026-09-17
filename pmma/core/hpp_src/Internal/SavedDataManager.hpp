#pragma once

namespace PMMA::Internal {
class SavedDataManager {
private:
    std::string CheckAgain;

public:
    void Save();

    void Load();

    bool ShouldCheckForUpdates() {
        if (CheckAgain.empty())
            return true;

        try {
            int year = 0;
            int month = 0;
            int day = 0;

            std::istringstream date(CheckAgain);

            char separator1;
            char separator2;

            date >> year >> separator1 >> month >> separator2 >> day;

            if (date.fail() ||
                separator1 != '-' ||
                separator2 != '-') {
                return true;
            }

            std::tm checkAgainTm{};

            checkAgainTm.tm_year = year - 1900;
            checkAgainTm.tm_mon = month - 1;
            checkAgainTm.tm_mday = day;
            checkAgainTm.tm_hour = 0;
            checkAgainTm.tm_min = 0;
            checkAgainTm.tm_sec = 0;

            const std::time_t checkAgainTime =
                std::mktime(&checkAgainTm);

            const std::time_t now =
                std::time(nullptr);

            return now >= checkAgainTime;
        } catch (...) {
            return true;
        }
    }
};
} // namespace PMMA::Internal