#pragma once

#include "sys/stat.h"
#include <algorithm>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"

#include "ModelEngineCommon.h"
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
    using embeddedpenguins::modelengine::ConfigurationUtilities;

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
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELCARRIERTYPE, class RECORDTYPE>
    class ModelRunner
    {
        bool valid_ { false };
        string controlFile_ {};

        ConfigurationUtilities configuration_ {};
        unique_ptr<ModelEngine<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>> modelEngine_ {};
        string modelInitializerLocation_ {};

    public:
        ModelEngine<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& GetModelEngine() { return *modelEngine_.get(); }
        const ConfigurationUtilities& ConfigurationCarrier() const { return configuration_; }
        const json& Control() const { return configuration_.Control(); }
        const json& Configuration() const { return configuration_.Configuration(); }
        const json& Monitor() const { return configuration_.Monitor(); }
        const json& Settings() const { return configuration_.Settings(); }
        const microseconds EnginePeriod() const { return modelEngine_->EnginePeriod(); }
        microseconds& EnginePeriod() { return modelEngine_->EnginePeriod(); }

    public:
        ModelRunner(int argc, char* argv[])
        {
            ParseArgs(argc, argv);

            if (valid_)
                valid_ = configuration_.InitializeConfiguration(controlFile_);

            if (valid_)
                modelInitializerLocation_ = configuration_.Configuration()["Execution"]["InitializerLocation"].get<string>();
        }

        //
        // Ensure the model is created and initialized, then start
        // it running asynchronously.
        //
        bool Run(MODELCARRIERTYPE& carrier)
        {
            if (!valid_)
                return false;

            return RunModelEngine(carrier);
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
        // Call Quit() and wait until the model engine stops.
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

        bool RunModelEngine(MODELCARRIERTYPE& carrier)
        {
            // Create the proxy with a two-step ctor-create sequence.
            ModelInitializerProxy<OPERATORTYPE, MODELCARRIERTYPE, RECORDTYPE> initializer(modelInitializerLocation_);
            initializer.CreateProxy(carrier, configuration_);

            // Let the initializer initialize the model's static state.
            initializer.Initialize();

            // Create and run the model engine.
            auto modelTicks = configuration_.Configuration()["Model"]["ModelTicks"];
            auto ticks { 1000 };
            if (modelTicks.is_number_integer() || modelTicks.is_number_unsigned())
                ticks = modelTicks.get<int>();

            modelEngine_ = make_unique<ModelEngine<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>>(
                carrier, 
                microseconds(ticks),
                configuration_);

            modelEngine_->RecordFile(configuration_.ComposeRecordPath());
            modelEngine_->LogFile(configuration_.ExtractRecordDirectory() + modelEngine_->LogFile());
            modelEngine_->Run();

            return true;
        }
    };
}
