#pragma once

#include <chrono>

#include "ModelEngineCommon.h"
#include "WorkerContext.h"
#include "WorkItem.h"

namespace embeddedpenguins::modelengine::threads
{
    using Clock = std::chrono::high_resolution_clock;
    using std::chrono::microseconds;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;

    using embeddedpenguins::modelengine::WorkItem;

    //
    // An instance of this class can be created with the worker context,
    // then passed to the client model code.  When the client code
    // determines new work to be done, it simply passes the new work
    // to the functor operator to become queued for future execution.
    //
    template<class OPERATORTYPE, class RECORDTYPE>
    class ProcessCallback
    {
        WorkerContext<OPERATORTYPE, RECORDTYPE>& context_;

    public:
        ProcessCallback() = delete;
        ProcessCallback(WorkerContext<OPERATORTYPE, RECORDTYPE>& context) :
            context_(context)
        {
            
        }

        void operator() (const OPERATORTYPE& work, int tickDelay = 0)
        {
            context_.WorkForNextThread.push_back(WorkItem<OPERATORTYPE> { context_.Iterations + tickDelay, work });
        }
    };
}
