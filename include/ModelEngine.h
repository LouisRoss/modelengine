#pragma once

#include <thread>
#include <chrono>

#include "ModelEngineContextOp.h"
#include "ModelEngineThread.h"
#include "sdk/ModelInitializerProxy.h"
#include "ProcessCallback.h"

namespace embeddedpenguins::modelengine
{
    using std::thread;
    using std::lock_guard;
    using std::chrono::microseconds;
    using std::chrono::nanoseconds;
    using std::chrono::high_resolution_clock;
    using time_point = std::chrono::high_resolution_clock::time_point;

    using embeddedpenguins::modelengine::threads::ProcessCallback;
    using embeddedpenguins::core::neuron::model::ModelInitializerProxy;

    //
    // The top-level control engine for running a model.
    // Create and run a single thread, which will create N worker objects, each
    // with its own thread.  By default, N will be the number of hardware cores - 1
    // so that between the model engine thread and the worker thread, all cores
    // will be kept busy.
    //
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELHELPERTYPE, class RECORDTYPE>
    class ModelEngine
    {
        ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE> context_;
        ModelEngineContextOp<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE> contextOp_;
        thread workerThread_;
        PartitionPolicy partitionPolicy_ { PartitionPolicy::AdaptiveWidth };
        nanoseconds duration_ {};
        time_point startTime_ {};

    public:
        const int GetWorkerCount() const { return context_.WorkerCount; }
        const long long int GetTotalWork() const { return context_.TotalWork; }
        const long long int GetIterations() const { return context_.Iterations; }
        const nanoseconds GetDuration() const { return duration_; }
        const string& LogFile() const { return context_.LogFile; }
        void LogFile(const string& logfile) { context_.LogFile = logfile; }
        const string& RecordFile() const { return context_.RecordFile; }
        void RecordFile(const string& recordfile) { context_.RecordFile = recordfile; }
        const microseconds EnginePeriod() const { return context_.EnginePeriod; }
        microseconds& EnginePeriod() { return context_.EnginePeriod; }

    public:
        ModelEngine() = delete;

        ModelEngine(MODELHELPERTYPE& helper, const ConfigurationRepository& configuration) :
            context_(configuration, helper),
            contextOp_(context_)
        {
            CreateWorkerThread(helper);
        }

        ~ModelEngine()
        {
            WaitForQuit();

            cout << "Model Engine stopping\n";
        }

    public:
        void Run()
        {
            startTime_ = high_resolution_clock::now();
            context_.Run = true;

            while (!context_.EngineInitialized && !context_.EngineInitializeFailed)
                std::this_thread::yield();

            if (context_.EngineInitializeFailed)
                WaitForQuit();
        }

        void Quit()
        {
            contextOp_.SignalQuit();
        }

        void WaitForQuit()
        {
            auto endTime = high_resolution_clock::now();
            duration_ = endTime - startTime_;

            contextOp_.SignalQuit();

            if (workerThread_.joinable())
                workerThread_.join();
        }

    private:
        void CreateWorkerThread(MODELHELPERTYPE& helper)
        {
            unique_ptr<IModelEnginePartitioner> partitioner { };
            switch (partitionPolicy_)
            {
            case PartitionPolicy::ConstantWidth:
                partitioner = make_unique<ConstantWidthPartitioner<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>>(context_);
                break;
            
            case PartitionPolicy::AdaptiveWidth:
                partitioner = make_unique<AdaptiveWidthPartitioner<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>>(context_);
                break;
            
            default:
                break;
            }

            unique_ptr<IModelEngineWaiter> waiter = make_unique<ConstantTickWaiter<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>>(context_);
            workerThread_ = thread(ModelEngineThread<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>(context_, helper, partitioner, waiter));
        }
    };
}
