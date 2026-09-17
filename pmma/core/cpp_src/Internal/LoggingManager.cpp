#ifdef USE_PYTHON
#include <Python.h>
#endif

#include <filesystem>
#include <functional>
#include <regex>

#include "Constants.hpp"
#include "Internal/Core/PMMA_Core.hpp"

struct LogFileEntry {
    std::filesystem::path path;
    time_t timestamp;
};

tm parseTimestamp(const std::string &name) {
    tm tm = {};
    sscanf(name.c_str(), "%2d-%2d-%4d at %2d-%2d-%2d",
           &tm.tm_mday, &tm.tm_mon, &tm.tm_year,
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_mon -= 1;     // tm_mon is 0-based
    tm.tm_year -= 1900; // tm_year is years since 1900
    return tm;
}

void ClearOldLogs(std::string LogDirectory, unsigned int KeepCount) {
    std::regex pattern(R"((\d{2}-\d{2}-\d{4} at \d{2}-\d{2}-\d{2})\.txt)");
    std::vector<LogFileEntry> files;

    for (const auto &entry : std::filesystem::directory_iterator(LogDirectory)) {
        if (!entry.is_regular_file())
            continue;

        std::smatch match;
        std::string filename = entry.path().filename().string();

        if (regex_match(filename, match, pattern)) {
            tm tm = parseTimestamp(match[1]);
            time_t ts = mktime(&tm);
            files.push_back({entry.path(), ts});
        }
    }

    // Sort by timestamp descending
    sort(files.begin(), files.end(), [](const LogFileEntry &a, const LogFileEntry &b) {
        return a.timestamp > b.timestamp;
    });

    // Delete files beyond the most recent `keep_count`
    for (size_t i = KeepCount; i < files.size(); ++i) {
        std::cout << "Deleting: " << files[i].path << "\n";
        try {
            std::filesystem::remove(files[i].path);
        } catch (const std::filesystem::filesystem_error &e) {
            std::cout << "Error deleting file: " << e.what() << "\n";
        }
    }
}

PMMA::Internal::LoggingManager::LoggingManager() {
    LogDebug = PMMA::Core::Registry::IsDebuggingModeEnabled;
}

PMMA::Internal::LoggingManager::~LoggingManager() {
    if (LogToFile && std::filesystem::exists(LogFileLocation)) {
        ClearOldLogs(LogFileLocation, KeepCount);
    }
}

void PMMA::Internal::LoggingManager::SetLogToFile(bool NewLogToFile) {
    LogToFile = NewLogToFile;
    LogToFileSpecifiedByUser = true;

    if (std::filesystem::exists(LogFileLocation)) {
        return;
    }

    if (PMMA::Core::PassportInstance != nullptr) {
        std::string ProductPath = PMMA::Core::PassportInstance->GetProductPath();

        if (LogToFile && std::filesystem::exists(ProductPath)) {
            try {
                std::filesystem::create_directory(ProductPath + PMMA::Core::Registry::PathSeparator + "logs");
                LogFileLocation = ProductPath + PMMA::Core::Registry::PathSeparator + "logs";
                FileCatchUp();
            } catch (const std::filesystem::filesystem_error &e) {
                PMMA::Core::LoggingManagerInstance->InternalLogError(
                    11,
                    "An error occurred whilst trying to create the \
directory: '" + ProductPath +
                        PMMA::Core::Registry::PathSeparator + "logs" + "'. \
The error details are: " +
                        e.what());
            }

            return;
        }
    }
    PMMA::Core::LoggingManagerInstance->InternalLogWarn(
        10,
        "No logging location has been set. Please use: \
`Passport.set_logging_location` to directly set a directory for the logs \
to be stored, or use `Passport.set_product_path` to allow PMMA to automatically \
manage a log file directory in your application. Until such a time, logs \
cannot be stored and only displayed at runtime.");
    LogToFile = false;
    LogToFileSpecifiedByUser = false;
}

void PMMA::Internal::LoggingManager::Log(std::string_view Type, std::string Timestamp, std::string Content) {
    if (LogToConsole) {
        std::string formattedContent;

        if (PMMA::Core::Registry::TerminalSupportsColor) {
            if (Type == PMMA::Constants::Logging_Types::LOG_INFO) {
                formattedContent = "PMMA - " + std::string(PMMA::Constants::ANSI_Escape_Codes::GREEN) + std::string(Type) + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET) + " - " + Timestamp + " - " + Content;
            } else if (Type == PMMA::Constants::Logging_Types::LOG_WARN) {
                formattedContent = "PMMA - " + std::string(PMMA::Constants::ANSI_Escape_Codes::YELLOW) + std::string(Type) + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET) + " - " + Timestamp + " - " + Content;
            } else if (Type == PMMA::Constants::Logging_Types::LOG_ERROR) {
                formattedContent = "PMMA - " + std::string(PMMA::Constants::ANSI_Escape_Codes::RED) + std::string(Type) + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET) + " - " + Timestamp + " - " + Content;
            } else {
                formattedContent = "PMMA - " + std::string(Type) + " - " + Timestamp + " - " + Content; // debug has no color
            }

            if (BoldFormatting) {
                formattedContent = std::string(PMMA::Constants::ANSI_Escape_Codes::STYLE_BOLD) + formattedContent + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET);
            }
        } else {
            formattedContent = "PMMA - " + std::string(Type) + " - " + Timestamp + " - " + Content;
        }

#ifdef USE_PYTHON
        PyGILState_STATE gstate = PyGILState_Ensure();
        PySys_WriteStdout("%s\n", formattedContent.c_str());
        PyGILState_Release(gstate);
#else
        std::cout << formattedContent << std::endl;
#endif

        BoldFormatting = !BoldFormatting; // Toggle bold formatting for console output
    }

    if (LogToFile) {
        std::ofstream LogFile(LogFileLocation + PMMA::Core::Registry::PathSeparator + LogFileName, std::ios::app);
        LogFile << "PMMA - " + std::string(Type) + " - " + Timestamp + " - " + Content << std::endl;
        LogFile.close();
    } else {
        ContentToLogToFile.push_back(Content);
    }
}

void PMMA::Internal::LoggingManager::Log(std::string Name, std::string_view Type, std::string Timestamp, std::string Content) {
    if (LogToConsole) {
        std::string formattedContent;

        if (PMMA::Core::Registry::TerminalSupportsColor) {
            if (Type == PMMA::Constants::Logging_Types::LOG_INFO) {
                formattedContent = Name + " - " + std::string(PMMA::Constants::ANSI_Escape_Codes::GREEN) + std::string(Type) + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET) + " - " + Timestamp + " - " + Content;
            } else if (Type == PMMA::Constants::Logging_Types::LOG_WARN) {
                formattedContent = Name + " - " + std::string(PMMA::Constants::ANSI_Escape_Codes::YELLOW) + std::string(Type) + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET) + " - " + Timestamp + " - " + Content;
            } else if (Type == PMMA::Constants::Logging_Types::LOG_ERROR) {
                formattedContent = Name + " - " + std::string(PMMA::Constants::ANSI_Escape_Codes::RED) + std::string(Type) + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET) + " - " + Timestamp + " - " + Content;
            } else {
                formattedContent = Name + " - " + std::string(Type) + " - " + Timestamp + " - " + Content; // debug has no color
            }

            if (BoldFormatting) {
                formattedContent = std::string(PMMA::Constants::ANSI_Escape_Codes::STYLE_BOLD) + formattedContent + std::string(PMMA::Constants::ANSI_Escape_Codes::RESET);
            }
        } else {
            formattedContent = Name + " - " + std::string(Type) + " - " + Timestamp + " - " + Content;
        }

#ifdef USE_PYTHON
        PyGILState_STATE gstate = PyGILState_Ensure();
        PySys_WriteStdout("%s\n", formattedContent.c_str());
        PyGILState_Release(gstate);
#else
        std::cout << formattedContent << std::endl;
#endif

        BoldFormatting = !BoldFormatting; // Toggle bold formatting for console output
    }

    if (LogToFile) {
        std::ofstream LogFile(LogFileLocation + PMMA::Core::Registry::PathSeparator + LogFileName, std::ios::app);
        LogFile << "PMMA - " + std::string(Type) + " - " + Timestamp + " - " + Content << std::endl;
        LogFile.close();
    } else {
        ContentToLogToFile.push_back(Content);
    }
}

void PMMA::Internal::LoggingManager::FileCatchUp() {
    if (LogToFile && ContentToLogToFile.size() > 0) {
        std::string fullPath = LogFileLocation + PMMA::Core::Registry::PathSeparator + LogFileName;

        std::ifstream inputFile(fullPath);
        std::string originalContents;
        if (inputFile.is_open()) {
            std::ostringstream buffer;
            buffer << inputFile.rdbuf();
            originalContents = buffer.str();
            inputFile.close();
        }

        std::ofstream outputFile(fullPath, std::ios::trunc);
        if (outputFile.is_open()) {
            for (const auto &line : ContentToLogToFile) {
                outputFile << line << std::endl;
            }

            outputFile << originalContents;
            outputFile.close();
        }

        ContentToLogToFile.clear();
    }
}

void PMMA::Internal::LoggingManager::SetLogFileLocation(std::string NewLogFileLocation) {
    LogFileLocation = NewLogFileLocation;
    LogFileName = GenerateLogFileName() + ".txt";

    FileCatchUp();
}

bool PMMA::Internal::LoggingManager::InternalLogDebug(int ID, std::string Content, bool RepeatForEffect) {
    if (!LogDebug) {
        return false;
    }

    if (PMMA::Core::Registry::IsDebuggingModeEnabled) {
        if (!RepeatForEffect) {
            auto PreviousIndex = find(PreviouslyLoggedContent.begin(), PreviouslyLoggedContent.end(), ID);
            if (PreviousIndex == PreviouslyLoggedContent.end()) {
                PreviouslyLoggedContent.push_back(ID);
                std::string DateTimeCode = GetDateTimeCode();
                Log(PMMA::Constants::Logging_Types::LOG_DEBUG, DateTimeCode, Content);
                return true;
            }
        } else {
            std::string DateTimeCode = GetDateTimeCode();
            Log(PMMA::Constants::Logging_Types::LOG_DEBUG, DateTimeCode, Content);
            return true;
        }
    }

    return false;
}

bool PMMA::Internal::LoggingManager::ExternalLogDebug(std::string ID, std::string Content, std::string ProductName, bool RepeatForEffect) {
    if (!LogDebug) {
        return false;
    }

    if (PMMA::Core::Registry::IsDebuggingModeEnabled) {
        transform(ProductName.begin(), ProductName.end(), ProductName.begin(), ::tolower);
        if (ProductName == "pmma") {
            PMMA::Core::LoggingManagerInstance->InternalLogError(
                57,
                "Failed to log debug message: The name PMMA or pmma is reserved!");

            throw std::runtime_error("The name PMMA or pmma is reserved!");
        }

        if (!RepeatForEffect) {
            int InternalID;
            transform(ID.begin(), ID.end(), ID.begin(), ::tolower);
            std::hash<std::string> hasher;
            InternalID = -(int)(hasher(ID));

            auto PreviousIndex = find(PreviouslyLoggedContent.begin(), PreviouslyLoggedContent.end(), InternalID);
            if (PreviousIndex == PreviouslyLoggedContent.end()) {
                PreviouslyLoggedContent.push_back(InternalID);
                std::string DateTimeCode = GetDateTimeCode();
                if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
                    ProductName = PMMA::Core::PassportInstance->ProductName + " ";
                }
                Log(ProductName, PMMA::Constants::Logging_Types::LOG_DEBUG, DateTimeCode, Content);
                return true;
            }
        } else {
            std::string DateTimeCode = GetDateTimeCode();
            if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
                ProductName = PMMA::Core::PassportInstance->ProductName + " ";
            }
            Log(ProductName, PMMA::Constants::Logging_Types::LOG_DEBUG, DateTimeCode, Content);
            return true;
        }
    }

    return false;
}

void PMMA::Internal::LoggingManager::ExternalLogInfo(std::string ID, std::string Content, std::string ProductName, bool RepeatForEffect) {
    if (!LogInfo) {
        return;
    }

    transform(ProductName.begin(), ProductName.end(), ProductName.begin(), ::tolower);
    if (ProductName == "pmma") {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            57,
            "Failed to log debug message: The name PMMA or pmma is reserved!");

        throw std::runtime_error("The name PMMA or pmma is reserved!");
    }

    if (!RepeatForEffect) {
        int InternalID;
        transform(ID.begin(), ID.end(), ID.begin(), ::tolower);
        std::hash<std::string> hasher;
        InternalID = -(int)(hasher(ID));

        auto PreviousIndex = find(PreviouslyLoggedContent.begin(), PreviouslyLoggedContent.end(), InternalID);
        if (PreviousIndex == PreviouslyLoggedContent.end()) {
            PreviouslyLoggedContent.push_back(InternalID);
            std::string DateTimeCode = GetDateTimeCode();
            if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
                ProductName = PMMA::Core::PassportInstance->ProductName + " ";
            }
            Log(ProductName, PMMA::Constants::Logging_Types::LOG_INFO, DateTimeCode, Content);
        }
    } else {
        std::string DateTimeCode = GetDateTimeCode();
        if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
            ProductName = PMMA::Core::PassportInstance->ProductName + " ";
        }
        Log(ProductName, PMMA::Constants::Logging_Types::LOG_INFO, DateTimeCode, Content);
    }
}

void PMMA::Internal::LoggingManager::ExternalLogWarn(std::string ID, std::string Content, std::string ProductName, bool RepeatForEffect) {
    if (!LogWarn) {
        return;
    }

    transform(ProductName.begin(), ProductName.end(), ProductName.begin(), ::tolower);
    if (ProductName == "pmma") {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            57,
            "Failed to log debug message: The name PMMA or pmma is reserved!");

        throw std::runtime_error("The name PMMA or pmma is reserved!");
    }

    if (!RepeatForEffect) {
        int InternalID;
        transform(ID.begin(), ID.end(), ID.begin(), ::tolower);
        std::hash<std::string> hasher;
        InternalID = -(int)(hasher(ID));

        auto PreviousIndex = find(PreviouslyLoggedContent.begin(), PreviouslyLoggedContent.end(), InternalID);
        if (PreviousIndex == PreviouslyLoggedContent.end()) {
            PreviouslyLoggedContent.push_back(InternalID);
            std::string DateTimeCode = GetDateTimeCode();
            if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
                ProductName = PMMA::Core::PassportInstance->ProductName + " ";
            }
            Log(ProductName, PMMA::Constants::Logging_Types::LOG_WARN, DateTimeCode, Content);
        }
    } else {
        std::string DateTimeCode = GetDateTimeCode();
        if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
            ProductName = PMMA::Core::PassportInstance->ProductName + " ";
        }
        Log(ProductName, PMMA::Constants::Logging_Types::LOG_WARN, DateTimeCode, Content);
    }
}

void PMMA::Internal::LoggingManager::ExternalLogError(std::string ID, std::string Content, std::string ProductName, bool RepeatForEffect) {
    if (!LogError) {
        return;
    }

    transform(ProductName.begin(), ProductName.end(), ProductName.begin(), ::tolower);
    if (ProductName == "pmma") {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            57,
            "Failed to log debug message: The name PMMA or pmma is reserved!");

        throw std::runtime_error("The name PMMA or pmma is reserved!");
    }

    if (!RepeatForEffect) {
        int InternalID;
        transform(ID.begin(), ID.end(), ID.begin(), ::tolower);
        std::hash<std::string> hasher;
        InternalID = -(int)(hasher(ID));

        auto PreviousIndex = find(PreviouslyLoggedContent.begin(), PreviouslyLoggedContent.end(), InternalID);
        if (PreviousIndex == PreviouslyLoggedContent.end()) {
            PreviouslyLoggedContent.push_back(InternalID);
            std::string DateTimeCode = GetDateTimeCode();
            if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
                ProductName = PMMA::Core::PassportInstance->ProductName + " ";
            }
            Log(ProductName, PMMA::Constants::Logging_Types::LOG_ERROR, DateTimeCode, Content);
        }
    } else {
        std::string DateTimeCode = GetDateTimeCode();
        if (ProductName == "" && PMMA::Core::PassportInstance->IsRegistered) {
            ProductName = PMMA::Core::PassportInstance->ProductName + " ";
        }
        Log(ProductName, PMMA::Constants::Logging_Types::LOG_ERROR, DateTimeCode, Content);
    }
}