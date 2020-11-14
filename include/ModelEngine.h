#pragma once

#include <thread>
#include <chrono>

#include "nlohmann/json.hpp"

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

    using nlohmann::json;

    using embeddedpenguins::modelengine::threads::ProcessCallback;
    using embeddedpenguins::modelengine::sdk::ModelInitializerProxy;

    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class ModelEngine
    {
        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE> context_;
        ModelEngineContextOp<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE> contextOp_;
        thread workerThread_;
        PartitionPolicy partitionPolicy_ { PartitionPolicy::AdaptiveWidth };
        nanoseconds duration_ {};
        time_point startTime_ {};

    public:
        int GetWorkerCount() { return context_.WorkerCount; }
        long long int GetTotalWork() { return context_.TotalWork; }
        long long int GetIterations() { return context_.Iterations; }
        nanoseconds GetDuration() { return duration_; }
        void SetLogFile(const string& logfile) { context_.LogFile = logfile; }
        void SetRecordFile(const string& recordfile) { context_.RecordFile = recordfile; }

    public:
        ModelEngine() = delete;

        ModelEngine(vector<NODETYPE>& model, microseconds enginePeriod, const json& configuration, int segmentCount = 0) :
            contextOp_(context_)
        {
            context_.WorkerCount = segmentCount;
            context_.EnginePeriod = enginePeriod;
            context_.Configuration = configuration;

            CreateWorkerThread(model);
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

            while (!context_.EngineInitialized)
                std::this_thread::yield();
        }

        void InitializeModel(ModelInitializerProxy<NODETYPE, OPERATORTYPE, RECORDTYPE>& initializer)
        {
            lock_guard<mutex> lock(context_.PartitioningMutex);
            ProcessCallback callback(context_.ExternalWorkSource);
            
            initializer.InjectSignal(callback);
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
        void CreateWorkerThread(vector<NODETYPE>& model)
        {
            unique_ptr<IModelEnginePartitioner> partitioner { };
            switch (partitionPolicy_)
            {
            case PartitionPolicy::ConstantWidth:
                partitioner = make_unique<ConstantWidthPartitioner<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>>(context_);
                break;
            
            case PartitionPolicy::AdaptiveWidth:
                partitioner = make_unique<AdaptiveWidthPartitioner<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>>(context_);
                break;
            
            default:
                break;
            }

            unique_ptr<IModelEngineWaiter> waiter = make_unique<ConstantTickWaiter<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>>(context_);
            workerThread_ = thread(ModelEngineThread<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>(context_, model, partitioner, waiter));
        }
    };
}
