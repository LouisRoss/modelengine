#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>
#include <condition_variable>

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
    using embeddedpenguins::modelengine::threads::Worker;
    using embeddedpenguins::modelengine::threads::WorkerContext;

    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    struct ModelEngineContext
    {
        atomic<bool> Run { false };
        bool Quit{false};
        mutex Mutex;
        condition_variable Cv;
        mutex PartitioningMutex;

        Log Logger {};
        LogLevel LoggingLevel { LogLevel::Status };
        string LogFile {"ModelEngine.log"};
        string RecordFile {"ModelEngineRecord.csv"};

        vector<unique_ptr<Worker<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>>> Workers {};
        WorkerContext<OPERATORTYPE, RECORDTYPE> ExternalWorkSource { Iterations, EnginePeriod, LoggingLevel };
        int WorkerCount { 0 };
        microseconds EnginePeriod;
        atomic<bool> EngineInitialized { false };
        microseconds PartitionTime { };
        unsigned long long int Iterations { 0LL };
        long long int TotalWork { 0LL };
    };
}
