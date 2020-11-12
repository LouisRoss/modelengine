#pragma once

#include <iostream>
#include "WorkerContext.h"
#include "ProcessCallback.h"

namespace embeddedpenguins::modelengine::threads
{
    using std::cout;
    using std::unique_lock;
    using std::lock_guard;

    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class WorkerThread
    {
    public:
        WorkerThread() = default;

        void operator() (WorkerContext<OPERATORTYPE, RECORDTYPE>& context)
        {
            ProcessCallback<OPERATORTYPE, RECORDTYPE> callback(context);

            while (context.Code != WorkCode::Quit)
            {
                WaitForSignal(context);

                if (context.Code == WorkCode::InitialScan)
                {
                    auto derived = static_cast<IMPLEMENTATIONTYPE&>(*this);
                    derived.Initialize(context.Logger, context.Record, context.Iterations, callback);
                }
                else if (context.Code == WorkCode::Scan)
                {
                    auto derived = static_cast<IMPLEMENTATIONTYPE&>(*this);
                    derived.Process(context.Logger, context.Record, context.Iterations, context.WorkForThread.begin(), context.WorkForThread.end(), callback);
                }
                else if (context.Code == WorkCode::FinalScan)
                {
                    auto derived = static_cast<IMPLEMENTATIONTYPE&>(*this);
                    derived.Finalize(context.Logger, context.Record, context.Iterations);
                }

                SignalDone(context);
            }

            SignalDone(context);
        }

    private:
        void WaitForSignal(WorkerContext<OPERATORTYPE, RECORDTYPE>& context)
        {
            unique_lock<mutex> lock(context.Mutex);
            context.Cv.wait(lock, [&context]{ return context.CycleStart; });
            context.CycleStart = false;
        }

        void SignalDone(WorkerContext<OPERATORTYPE, RECORDTYPE>& context)
        {
            {
                lock_guard<mutex> lock(context.MutexReturn);
                context.CycleDone = true;
            }
            context.CvReturn.notify_one();
        }
    };
}
