#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <limits>

#include "ModelEngineCommon.h"
#include "ModelEngineContextOp.h"
#include "AdaptiveWidthPartitioner.h"
#include "ConstantWidthPartitioner.h"
#include "ConstantTickWaiter.h"
#include "Worker.h"
#include "Log.h"
#include "Recorder.h"

namespace embeddedpenguins::modelengine
{
    using std::vector;
    using std::unique_ptr;
    using std::make_unique;
    using std::unique_lock;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    using std::chrono::microseconds;
    using std::chrono::duration_cast;
    using std::numeric_limits;
    using std::cout;
    using std::cerr;

    using embeddedpenguins::modelengine::threads::Worker;
    using embeddedpenguins::modelengine::threads::WorkCode;

    //
    // The model engine does its work in this thread object.
    // The main loop of this thread object runs a two-phase process where
    // 1) this thread partitions existing work to the worker threads, and
    // 2) the worker threads execute the work.
    // The two phases do not run simultaneously, but use synchronization barriers
    // to ensure the phases run serially.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELCARRIERTYPE, class RECORDTYPE>
    class ModelEngineThread
    {
        unique_ptr<IModelEngineWaiter> waiter_ { };
        unique_ptr<IModelEnginePartitioner> partitioner_ { };

        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& context_;
        ModelEngineContextOp<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE> contextOp_;
        MODELCARRIERTYPE carrier_;

    public:
        ModelEngineThread() = delete;
        ModelEngineThread(
                        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& context, 
                        MODELCARRIERTYPE carrier, 
                        unique_ptr<IModelEnginePartitioner>& partitioner, 
                        unique_ptr<IModelEngineWaiter>& waiter) :
            context_(context),
            contextOp_(context),
            carrier_(carrier),
            waiter_(std::move(waiter)),
            partitioner_(std::move(partitioner))
        {
            context_.Logger.SetId(0);

            if (context_.WorkerCount == 0)
                context_.WorkerCount = std::thread::hardware_concurrency() - 1;
        }

        void operator() ()
        {
            while (!context_.Run) { std::this_thread::yield(); }

            try
            {
                Initialize();
                MainLoop();
                Cleanup();
            }
            catch(const std::exception& e)
            {
                cout << "ModelEngine exception while running: " << e.what() << '\n';
            }

            Log::Merge(context_.ExternalWorkSource.Logger);
            Log::Merge(context_.Logger);
            for (auto& worker : context_.Workers)
            {
                Log::Merge(worker->GetContext().Logger);
                Recorder<RECORDTYPE>::Merge(worker->GetContext().Record);
            }
            cout << "Writing log file to " << context_.LogFile << "... " << std::flush;
            Log::Print(context_.LogFile.c_str());
            cout << "Done\n";
            cout << "Writing record file to " << context_.RecordFile << "... " << std::flush;
            Recorder<RECORDTYPE>::Print(context_.RecordFile.c_str());
            cout << "Done\n";
        }

        unsigned long long int GetIterations()
        {
            return context_.Iterations;
        }

    private:
        void Initialize()
        {
            contextOp_.CreateWorkers(carrier_);

            context_.Workers[0]->Scan(WorkCode::InitialScan);
            context_.Workers[0]->WaitForPreviousScan();

            context_.Iterations = 0ULL;
            context_.EngineInitialized = true;
        }

        void MainLoop()
        {
            auto engineStartTime = high_resolution_clock::now();
#ifndef NOLOG
            context_.Logger.Logger() << "ModelEngine starting main loop\n";
            context_.Logger.Logit();
#endif

            auto quit {false};
            do
            {
                quit = WaitForWorkOrQuit();
                if (!quit)
                {
                    lock_guard<mutex> lock(context_.PartitioningMutex);
                    StartWorkWithAllWorkers();
                    partitioner_->ConcurrentPartitionStep();
                    WaitForAllWorkersToCompleteWork();
                    PartitionWork();

                    SwitchWorkingBuffersForAllWorkers();
                    ++context_.Iterations;
                } 
            }
            while (!quit);
            auto engineElapsed = duration_cast<microseconds>(high_resolution_clock::now() - engineStartTime).count();
            auto partitionElapsed = context_.PartitionTime.count();

#ifndef NOLOG
            context_.Logger.Logger() << "ModelEngine quitting main loop\n";
            context_.Logger.Logit();
#endif

            double partitionRatio = (double)partitionElapsed / (double)engineElapsed;
            cout 
                << "Iterations: " << context_.Iterations 
                << " Total Work: " << context_.TotalWork 
                << " items  Partition Time: " << partitionElapsed << '/' << engineElapsed << " us = " 
                << partitionRatio 
                << "\n";
        }

        bool WaitForWorkOrQuit()
        {
            if (waiter_) return waiter_->WaitForWorkOrQuit();
            return true;
        }

        void StartWorkWithAllWorkers()
        {
#ifndef NOLOG
            if (context_.LoggingLevel == LogLevel::Diagnostic)
            {
                auto workPresent { false };
                for (auto& worker : context_.Workers)
                    workPresent |= worker->GetContext().WorkForThread.size() > 0;

                if (workPresent)
                {
                    context_.Logger.Logger() << "Starting worker threads [ ";
                    for (auto& worker : context_.Workers)
                    {
                        auto& workForThread = worker->GetContext().WorkForThread;
                        if (workForThread.size() < 2)
                        {
                            context_.Logger.Logger() << workForThread.size() << ' ';
                        }
                        else
                        {
                            context_.Logger.Logger() << workForThread.size() 
                                << '(' 
                                << workForThread.front().Operator.Index
                                << '-'
                                << workForThread.back().Operator.Index
                                << ") ";
                        }
                    }
                    context_.Logger.Logger() << "] for tick " << context_.Iterations << '\n';
                    context_.Logger.Logit();
                }
            }
            else if (context_.LoggingLevel != LogLevel::None)
            {
                context_.Logger.Logger() << "Starting worker threads\n";
                context_.Logger.Logit();
            }
#endif

            for (auto& worker : context_.Workers)
                worker->Scan(WorkCode::Scan);
        }

        void WaitForAllWorkersToCompleteWork()
        {
            for (auto& worker : context_.Workers)
                worker->WaitForPreviousScan();

#ifndef NOLOG
            context_.Logger.Logger() << "Worker threads complete\n";
            context_.Logger.Logit();
#endif
        }

        void PartitionWork()
        {
            auto partitionStartTime = high_resolution_clock::now();

            auto workForTick = partitioner_->SingleThreadPartitionStep();
            if (workForTick > 0)
            {
                auto partitionElapsed = high_resolution_clock::now() - partitionStartTime;
                context_.PartitionTime += duration_cast<microseconds>(partitionElapsed);
            }
        }

        void SwitchWorkingBuffersForAllWorkers()
        {
            for (auto& sourceWorker : context_.Workers)
            {
                auto& currentBuffer = sourceWorker->GetContext().CurrentBuffer;
                currentBuffer = (currentBuffer == CurrentBufferType::Buffer1Current ? 
                        CurrentBufferType::Buffer2Current : 
                        CurrentBufferType::Buffer1Current);
            }

            auto& currentExternalBuffer = context_.ExternalWorkSource.CurrentBuffer;
            currentExternalBuffer = (currentExternalBuffer == CurrentBufferType::Buffer1Current ? 
                    CurrentBufferType::Buffer2Current : 
                    CurrentBufferType::Buffer1Current);
        }

        void Cleanup()
        {
            context_.Workers[0]->Scan(WorkCode::FinalScan);
            context_.Workers[0]->WaitForPreviousScan();

#ifndef NOLOG
            context_.Logger.Logger() << "ModelEngine waiting for worker threads to quit\n";
            context_.Logger.Logit();
#endif
            for (auto& worker : context_.Workers)
                worker->Join();
#ifndef NOLOG
            context_.Logger.Logger() << "ModelEngine joined all worker threads\n";
            context_.Logger.Logit();
#endif
        }
   };
}
