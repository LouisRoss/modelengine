#pragma once

#include <iostream>
#include "WorkerContext.h"
#include "ProcessCallback.h"

namespace embeddedpenguins::modelengine::threads
{
    using std::cout;
    using std::unique_lock;
    using std::lock_guard;

    //
    // The client code implements the model algorithm by deriving a class
    // (typically called <modelName>Implementation) from this class.  The name
    // of the derived class must be passed in as the IMPLEMENTATIONTYPE template
    // parameter.
    // The thread main loop waits for a signal to run (coordinated by the ModelEngineThread),
    // and executes the path appropriate to the work code passed in through the
    // context object.
    // Each command results in a downcast to the derived client class, and
    // a call to the required method on that class.
    //
    // NOTE the derived class is expected to implement the required methods,
    //      but no interface exists to enforce the implementation.  Run time
    //      will be faster using compile-time polymorphism with templates rather
    //      than run-time polymorphism, as having the derived class override 
    //      virtual methods.
    //      In any case, if a required method is missing or malformed in the
    //      derived class, it will fail to compile with a similar error as
    //      would happen with an interface class.
    //
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
