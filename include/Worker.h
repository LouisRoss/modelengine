#pragma once

#include <thread>

#include "ModelEngineCommon.h"
#include "WorkerContext.h"
#include "WorkerThread.h"
#include "Log.h"

namespace embeddedpenguins::modelengine::threads
{
    using std::thread;
    using std::unique_lock;
    using std::lock_guard;

    using embeddedpenguins::modelengine::ConfigurationUtilities;

    //
    // One worker instance is created for each hardware thread
    // except for the main thread reserved for the ModelEngine.
    // The worker object uses synchronization objects in its context
    // to control its worker thread.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELCARRIERTYPE, class RECORDTYPE>
    class Worker
    {
        thread workerThread_;
        bool firstScan_ { true };
        WorkerContext<OPERATORTYPE, RECORDTYPE> context_;

    public:
        WorkerContext<OPERATORTYPE, RECORDTYPE>& GetContext() { return context_; }
        int GetWorkerId() { return context_.WorkerId; }

    public:
        Worker() = delete;

        Worker(MODELCARRIERTYPE& carrier, int workerId, microseconds& enginePeriod, 
                    const ConfigurationUtilities& configuration, 
                    unsigned long long int segmentStart, unsigned long long int segmentEnd, 
                    unsigned long long int& iterations, 
                    LogLevel& loggingLevel) :
            context_(iterations, enginePeriod, loggingLevel)
        {
            context_.Logger.SetId(workerId);
            context_.WorkerId = workerId;
            context_.RangeBegin = segmentStart;
            context_.RangeEnd = segmentEnd;

            workerThread_ = thread(IMPLEMENTATIONTYPE(workerId, carrier, configuration), std::ref(context_));
        }

        void Scan(WorkCode code)
        {
            WaitForPreviousScan();
            SetDataForScan(code);

            context_.Cv.notify_one();
        }

        void WaitForPreviousScan()
        {
            if (firstScan_)
            {
                firstScan_ = false;
                return;
            }

            unique_lock<mutex> lock(context_.MutexReturn);
            context_.CvReturn.wait(lock, [this]{ return context_.CycleDone; });
        }

        void Join()
        {
            Scan(WorkCode::Quit);
            workerThread_.join();
        }

        ~Worker()
        {
            if (workerThread_.joinable()) workerThread_.join();
        }

    private:
        void SetDataForScan(WorkCode code)
        {
            lock_guard<mutex> lock(context_.Mutex);
            context_.Code = code;
            context_.CycleDone = false;
            context_.CycleStart = true;
        }
    };
}
