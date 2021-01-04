#pragma once

#include <chrono>
#include <limits>

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

        //
        // Push a new work item onto the big list.  The tickDelay is relative
        // to the current tick (this is typically called by work in progress),
        // so zero is invalid -- it is not possible to schedule any work for 
        // the current tick.  Treat anything less than 1 as 1.
        //
        void operator() (const OPERATORTYPE& work, int tickDelay = 1)
        {
            if (tickDelay <= 1)
            {
                context_.WorkForTick1.push_back(WorkItem<OPERATORTYPE> { context_.Iterations + 1, work });
                return;
            }

            // As the worker thread, use the current buffer.  The partitioner will use the other one.
            if (context_.CurrentBuffer == CurrentBufferType::Buffer1Current)
            {
                context_.WorkForFutureTicks1.push_back(WorkItem<OPERATORTYPE> { context_.Iterations + tickDelay - 1, work });
            }
            else
            {
                context_.WorkForFutureTicks2.push_back(WorkItem<OPERATORTYPE> { context_.Iterations + tickDelay - 1, work });
            }
        }
    };
}
