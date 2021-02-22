#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>
#include <condition_variable>

#include "ConfigurationRepository.h"

#include "Worker.h"
#include "WorkerContext.h"
#include "Log.h"

namespace embeddedpenguins::modelengine
{
    using std::string;
    using std::atomic;
    using std::mutex;
    using std::condition_variable;
    using std::vector;
    using std::unique_ptr;
    using std::chrono::microseconds;

    using embeddedpenguins::core::neuron::model::ConfigurationRepository;

    using embeddedpenguins::core::neuron::model::Log;
    using embeddedpenguins::core::neuron::model::LogLevel;

    using embeddedpenguins::modelengine::threads::Worker;
    using embeddedpenguins::modelengine::threads::WorkerContext;

    //
    // Carry the public information defining the model engine.
    // This includes synchronization between the model engine and its thread;
    // configuration and logging; all workers; and statistics about the run.
    //
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELHELPERTYPE, class RECORDTYPE>
    struct ModelEngineContext
    {
        atomic<bool> Run { false };
        bool Quit { false };
        mutex Mutex;
        condition_variable Cv;
        mutex PartitioningMutex;

        ConfigurationRepository Configuration {};
        MODELHELPERTYPE& Helper;
        Log Logger {};
        LogLevel LoggingLevel { LogLevel::Status };
        string LogFile {"ModelEngine.log"};
        string RecordFile {"ModelEngineRecord.csv"};

        vector<unique_ptr<Worker<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>>> Workers {};
        WorkerContext<OPERATORTYPE, RECORDTYPE> ExternalWorkSource { Iterations, EnginePeriod, LoggingLevel };
        int WorkerCount { 0 };
        microseconds EnginePeriod;
        atomic<bool> EngineInitialized { false };
        atomic<bool> EngineInitializeFailed { false };
        microseconds PartitionTime { };
        unsigned long long int Iterations { 0LL };
        long long int TotalWork { 0LL };

        ModelEngineContext(const ConfigurationRepository& configuration, MODELHELPERTYPE& helper) :
            Configuration(configuration),
            Helper(helper),
            EnginePeriod(1000)
        {
            // Create and run the model engine.
            const json& modelJson = Configuration.Configuration()["Model"];
            if (modelJson.is_null()) return;

            const json& modelTicksJson = modelJson["ModelTicks"];
            if (modelTicksJson.is_number_integer() || modelTicksJson.is_number_unsigned())
                EnginePeriod = microseconds(modelTicksJson.get<int>());

            RecordFile = Configuration.ComposeRecordPath();
            LogFile = Configuration.ExtractRecordDirectory() + LogFile;
        }
    };
}
