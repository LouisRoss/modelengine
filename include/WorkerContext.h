#pragma once

#include <mutex>
#include <vector>
#include <chrono>
#include <condition_variable>

#include "Log.h"
#include "Recorder.h"
#include "ModelEngineCommon.h"
#include "WorkItem.h"

namespace embeddedpenguins::modelengine::threads
{
    using std::mutex;
    using std::vector;
    using std::pair;
    using std::condition_variable;
    using std::chrono::microseconds;
    using time_point = std::chrono::high_resolution_clock::time_point;
    using embeddedpenguins::modelengine::threads::WorkCode;
    using embeddedpenguins::modelengine::Log;
    using embeddedpenguins::modelengine::Recorder;
    using embeddedpenguins::modelengine::WorkItem;

    enum class CurrentBufferType
    {
        Buffer1Current,
        Buffer2Current
    };

    //
    // Carry the public information defining the worker.
    // This consists primarily of synchronization between the worker and its thread.
    // Some parameters such as iteration count and engine period are actually
    // references to parameters in the ModelEngineContext.
    //
    template<class OPERATORTYPE, class RECORDTYPE>
    struct WorkerContext
    {
        mutex Mutex;
        condition_variable Cv;
        mutex MutexReturn;
        condition_variable CvReturn;
        bool CycleStart{false};
        bool CycleDone{false};
        WorkCode Code{WorkCode::Run};
        microseconds& EnginePeriod;
        unsigned long long int& Iterations;

        int WorkerId {0};
        Log Logger {};
        LogLevel& LoggingLevel;
        Recorder<RECORDTYPE> Record;
        unsigned long long int RangeBegin{0LL};
        unsigned long long int RangeEnd{0LL};
        vector<WorkItem<OPERATORTYPE>> WorkForThread;
        vector<WorkItem<OPERATORTYPE>> WorkForTick1;
        CurrentBufferType CurrentBuffer { CurrentBufferType::Buffer1Current };
        vector<WorkItem<OPERATORTYPE>> WorkForFutureTicks1;
        vector<WorkItem<OPERATORTYPE>> WorkForFutureTicks2;

        WorkerContext(unsigned long long int& iterations, microseconds& enginePeriod, LogLevel& loggingLevel) : 
            Iterations(iterations), 
            Record(iterations), 
            EnginePeriod(enginePeriod), 
            LoggingLevel(loggingLevel)
        {
        }
    };
}
