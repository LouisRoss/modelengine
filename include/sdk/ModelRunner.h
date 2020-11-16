#pragma once

#include "sys/stat.h"
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "nlohmann/json.hpp"

#include "ModelEngine.h"
#include "ModelInitializerProxy.h"

namespace embeddedpenguins::modelengine::sdk
{
    using std::string;
    using std::begin;
    using std::end;
    using std::remove;
    using std::unique_ptr;
    using std::make_unique;
    using std::vector;
    using std::cout;
    using std::ifstream;
    using nlohmann::json;
    using embeddedpenguins::modelengine::ModelEngine;

    //
    // Wrap the most common startup and teardown sequences to run a model
    // in a single class.  A typical application can just instantiate a 
    // ModelRunner object and call its Run() method to start the model running.
    // Call its Quit() method to make the model stop at the end of the next tick.
    // Call its WaitForQuit() method to confirm that the model engine as stopped and
    // cleaned up.
    //
    // When using the ModelRunner to run a model, it owns the model (a vector of NODETYPE),
    // the model engine object, and all configuration defined for the model.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class ModelRunner
    {
        bool valid_ { false };
        string configurationPath_ {};
        string controlFile_ {};
        string configFile_ {};
        string monitorFile_ {};
        string settingsFile_ { "./ModelSettings.json" };
        json control_ {};
        json configuration_ {};
        json monitor_ {};
        json settings_ {};
        vector<NODETYPE> model_;
        unique_ptr<ModelEngine<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>> modelEngine_ {};
        string modelInitializerLocation_ {};

    public:
        ModelEngine<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& GetModelEngine() { return *modelEngine_.get(); }
        const json& Control() const { return control_; }
        const json& Configuration() const { return configuration_; }
        const json& Monitor() const { return monitor_; }
        const json& Settings() const { return settings_; }
        const microseconds EnginePeriod() const { return modelEngine_->EnginePeriod(); }
        microseconds& EnginePeriod() { return modelEngine_->EnginePeriod(); }
        vector<NODETYPE>& GetModel() { return model_; }

    public:
        ModelRunner(int argc, char* argv[])
        {
            ParseArgs(argc, argv);
            LoadSettings();

            if (valid_)
                LoadControl();

            if (valid_)
                LoadConfiguration();

            if (valid_)
                LoadMonitor();

            if (valid_)
                modelInitializerLocation_ = configuration_["Execution"]["InitializerLocation"].get<string>();
        }

        //
        // Ensure the model is created and initialized, then start
        // it running asynchronously.
        //
        bool Run()
        {
            if (!valid_)
                return false;

            return RunModelEngine();
        }

        //
        // Start an async process to stop the model engine
        // and return immediately.  To guarantee it has stopped,
        // call WaitForQuit().
        //
        void Quit()
        {
            modelEngine_->Quit();
        }

        //
        // Cal Quit() and wait until the model engine stops.
        // It is legal to call this after Quit().
        //
        void WaitForQuit()
        {
            modelEngine_->WaitForQuit();
        }

    private:
        void ParseArgs(int argc, char *argv[])
        {
            static string usage {
                " <control file>\n"
                "  <control file> is the name of the json file "
                "containing the control information (configuration"
                "and monitor) for the test to run.\n"
            };

            if (argc < 2)
            {
                cout << "Usage: " << argv[0] << usage;
                return;
            }

            for (auto i = 1; i < argc; i++)
            {
                const auto& arg = argv[i];
                if (arg[0] == '-') continue;
                controlFile_ = arg;
            }

            if (controlFile_.empty())
            {
                cout << "Usage: " << argv[0] << usage;
                return;
            }

            if (controlFile_.length() < 5 || controlFile_.substr(controlFile_.length()-5, controlFile_.length()) != ".json")
                controlFile_ += ".json";

            cout << "Using control file " << controlFile_ << "\n";

            valid_ = true;
        }

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

        bool RunModelEngine()
        {
            // Create the proxy with a two-step ctor-create sequence.
            ModelInitializerProxy<NODETYPE, OPERATORTYPE, RECORDTYPE> initializer(modelInitializerLocation_);
            initializer.CreateProxy(model_, configuration_);

            // Let the initializer initialize the model's static state.
            initializer.Initialize();

            // Create and run the model engine.
            auto modelTicks = configuration_["Model"]["ModelTicks"];
            auto ticks { 1000 };
            if (modelTicks.is_number_integer() || modelTicks.is_number_unsigned())
                ticks = modelTicks.get<int>();

            modelEngine_ = make_unique<ModelEngine<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>>(
                model_, 
                microseconds(ticks),
                configuration_);

            modelEngine_->SetRecordFile(ComposeRecordPath());
            modelEngine_->Run();

            // After the model is running, let the initializer inject a startup work load.
            modelEngine_->InitializeModel(initializer);

            return true;
        }

        string ComposeRecordPath()
        {
            string location(configuration_["PostProcessing"]["RecordLocation"]);
            string file(configuration_["PostProcessing"]["RecordFile"]);
            auto recordPath = location + file;
            recordPath.erase(remove(begin(recordPath), end(recordPath), '\"'), end(recordPath));

            return recordPath;
        }
    };
}
