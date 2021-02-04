#pragma once

#include "sys/stat.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "nlohmann/json.hpp"

namespace embeddedpenguins
{
    namespace modelengine::threads
    {
        enum class WorkCode
        {
            Run,
            Quit,
            Scan
        };
    }

    namespace modelengine
    {
        using std::cout;
        using std::string;
        using std::ifstream;
        using std::filesystem::create_directories;

        using nlohmann::json;
        
        enum class PartitionPolicy
        {
            ConstantWidth,
            AdaptiveWidth
        };

        class ConfigurationUtilities
        {
            bool valid_ { false };
            string configurationPath_ {};
            string settingsFile_ { "./ModelSettings.json" };
            string controlFile_ {};
            string configFile_ {};
            string monitorFile_ {};
            json configuration_ {};
            json control_ {};
            json monitor_ {};
            json settings_ {};

        public:
            const bool Valid() const { return valid_; }
            const json& Control() const { return control_; }
            const json& Configuration() const { return configuration_; }
            const json& Monitor() const { return monitor_; }
            const json& Settings() const { return settings_; }

        public:
            ConfigurationUtilities() = default;

            bool InitializeConfiguration(const string& controlFile)
            {
                controlFile_ = controlFile;
                valid_ = true;

                LoadSettings();

                if (valid_)
                    LoadControl();

                if (valid_)
                    LoadConfiguration();

                if (valid_)
                    LoadMonitor();

                return valid_;
            }

            const string ComposeRecordPath() const
            {
                auto recordDirectory = ExtractRecordDirectory();

                string fileName {"ModelEngineRecord.csv"};
                auto file(configuration_["PostProcessing"]["RecordFile"]);
                if (file.is_string())
                    fileName = file.get<string>();

                return recordDirectory + fileName;
            }

            const string ExtractRecordDirectory() const
            {
                string recordDirectory {"./"};
                auto path = configuration_["PostProcessing"]["RecordLocation"];
                if (path.is_string())
                    recordDirectory = path.get<string>();

                if (recordDirectory[recordDirectory.length() - 1] != '/')
                    recordDirectory += '/';

                auto project = control_["Configuration"];
                if (project.is_string())
                {
                    auto configuration= project.get<string>();
                    auto lastSlashPos = configuration.rfind('/');
                    if (lastSlashPos != configuration.npos)
                        configuration = configuration.substr(lastSlashPos + 1, configuration.size() - lastSlashPos);

                    auto jsonExtensionPos = configuration.rfind(".json");
                    if (jsonExtensionPos != configuration.npos)
                        configuration = configuration.substr(0, jsonExtensionPos);

                    recordDirectory += configuration;
                }

                if (recordDirectory[recordDirectory.length() - 1] != '/')
                    recordDirectory += '/';

                cout << "Record directory: " << recordDirectory << '\n';

                create_directories(recordDirectory);
                return recordDirectory;
            }

        private:
            void LoadSettings()
            {
                if (settingsFile_.length() < 5 || settingsFile_.substr(settingsFile_.length()-5, settingsFile_.length()) != ".json")
                    settingsFile_ += ".json";

                struct stat buffer;   
                if (!(stat (settingsFile_.c_str(), &buffer) == 0))
                {
                    cout << "Settings file " << settingsFile_ << " does not exist, using defaults\n";
                    return;
                }

                cout << "LoadSettings from " << settingsFile_ << "\n";
                ifstream settings(settingsFile_);
                settings >> settings_;

                configurationPath_ = "./";
                auto configFilePath = settings_["ConfigFilePath"];
                if (!configFilePath.is_null())
                {
                    configurationPath_ = string(configFilePath);
                    configurationPath_.erase(remove(begin(configurationPath_), end(configurationPath_), '\"'), end(configurationPath_));
                }
            }

            void LoadControl()
            {
                controlFile_ = configurationPath_ + "/" + controlFile_;
                cout << "Using control file " << controlFile_ << "\n";

                struct stat buffer;   
                if (!(stat (controlFile_.c_str(), &buffer) == 0))
                {
                    cout << "Control file " << controlFile_ << " does not exist\n";
                    valid_ = false;
                    return;
                }

                cout << "LoadControl from " << controlFile_ << "\n";
                ifstream control(controlFile_);
                control >> control_;
            }

            void LoadConfiguration()
            {
                configFile_ = control_["Configuration"];
                configFile_.erase(remove(begin(configFile_), end(configFile_), '\"'), end(configFile_));
                if (configFile_.length() < 5 || configFile_.substr(configFile_.length()-5, configFile_.length()) != ".json")
                    configFile_ += ".json";

                configFile_ = configurationPath_ + "/" + configFile_;
                cout << "Using config file " << configFile_ << "\n";

                struct stat buffer;   
                if (!(stat (configFile_.c_str(), &buffer) == 0))
                {
                    cout << "Configuration file " << configFile_ << " does not exist\n";
                    valid_ = false;
                    return;
                }

                cout << "LoadConfiguration from " << configFile_ << "\n";
                ifstream config(configFile_);
                config >> configuration_;
            }

            void LoadMonitor()
            {
                monitorFile_ = control_["Monitor"];
                monitorFile_.erase(remove(begin(monitorFile_), end(monitorFile_), '\"'), end(monitorFile_));
                if (monitorFile_.length() < 5 || monitorFile_.substr(monitorFile_.length()-5, monitorFile_.length()) != ".json")
                    monitorFile_ += ".json";

                monitorFile_ = configurationPath_ + "/" + monitorFile_;
                cout << "Using monitor file " << monitorFile_ << "\n";

                struct stat buffer;   
                if (!(stat (monitorFile_.c_str(), &buffer) == 0))
                {
                    // Non-fatal error, leave valid_ as-is.
                    cout << "Monitor file " << monitorFile_ << " does not exist\n";
                    return;
                }

                cout << "LoadMonitor from " << monitorFile_ << "\n";
                ifstream monitor(monitorFile_);
                monitor >> monitor_;
            }

        };
    }
}
