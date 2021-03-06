#pragma once

#include <iostream>
#include <chrono>
#include "ModelEngineCommon.h"
#include "ModelEngineContext.h"
#include "Worker.h"
#include "Log.h"

namespace embeddedpenguins::modelengine
{
    using std::cout;
    using std::lock_guard;
    using std::unique_lock;
    using std::make_unique;
    using time_point = std::chrono::high_resolution_clock::time_point;

    //
    // Separate the executable code from the context carrier object so that the
    // context carrier object may be passed around without exposing methods.
    // 
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELHELPERTYPE, class RECORDTYPE>
    class ModelEngineContextOp
    {
        ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>& context_;

    public:
        ModelEngineContextOp(ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>& context) :
            context_(context)
        {

        }

        void CreateWorkers(MODELHELPERTYPE& helper)
        {
            auto segmentSize = helper.Model().ModelSize() / context_.WorkerCount;
            long long int segmentStart { 0LL };
            for (auto id = 1; id < context_.WorkerCount; id++, segmentStart += segmentSize)
            {
                context_.Workers.push_back(
                    make_unique<Worker<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>>(
                        helper, 
                        id, 
                        context_.EnginePeriod, 
                        context_.Configuration,
                        segmentStart, 
                        segmentStart + segmentSize,
                        context_.Iterations,
                        context_.LoggingLevel));
            }
            context_.Workers.push_back(
                make_unique<Worker<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>>(
                    helper, 
                    context_.WorkerCount, 
                    context_.EnginePeriod, 
                    context_.Configuration,
                    segmentStart, 
                    helper.Model().ModelSize(),
                    context_.Iterations,
                    context_.LoggingLevel));

            context_.ExternalWorkSource.Logger.SetId(context_.WorkerCount + 1);
            context_.ExternalWorkSource.EnginePeriod = context_.EnginePeriod;
            context_.ExternalWorkSource.WorkerId = context_.WorkerCount + 1;
            context_.ExternalWorkSource.RangeBegin = 0LL;
            context_.ExternalWorkSource.RangeEnd = helper.Model().ModelSize();
        }

        void SignalQuit()
        {
            {
                lock_guard<mutex> lock(context_.Mutex);
                context_.Quit = true;
            }
            context_.Cv.notify_one();
        }

        bool WaitForWorkOrQuit(time_point time)
        {
            //context_.Logger.Logger() << "WaitForWorkOrQuit() waiting until " << Log::FormatTime(time) << "\n";
            //context_.Logger.Logit();
            unique_lock<mutex> lock(context_.Mutex);
            context_.Cv.wait_until(lock, time, [this](){ return context_.Quit; });

            return context_.Quit;
        }

        bool WaitForWorkOrQuit()
        {
            //context_.Logger.Logger() << "WaitForWorkOrQuit() waiting forever\n";
            //context_.Logger.Logit();
            unique_lock<mutex> lock(context_.Mutex);
            context_.Cv.wait(lock, [this](){ return context_.Quit; });

            return context_.Quit;
        }
    };
}
