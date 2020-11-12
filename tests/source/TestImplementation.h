#pragma once

#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "WorkerThread.h"
#include "WorkItem.h"
#include "ProcessCallback.h"
#include "Recorder.h"
#include "TestOperation.h"
#include "TestNode.h"
#include "TestRecord.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using ::embeddedpenguins::modelengine::threads::WorkerThread;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;
    using ::embeddedpenguins::modelengine::Log;
    using ::embeddedpenguins::modelengine::Recorder;
    using ::embeddedpenguins::modelengine::WorkItem;
    using std::vector;
    using std::pair;
    using std::for_each;
    using std::cout;
    using std::chrono::milliseconds;
    using time_point = std::chrono::high_resolution_clock::time_point;

    // Note: the callback should be allowed to be declared something like
    // void (*callback)(const SpikingOperation&)
    // but I could not get that to work.  As a workaround,
    // I have explicitly referenced the functor class used
    // by the template library (ProcessCallback<SpikingOperation>).
    // It would be great to get this to work so I could reduce the
    // dependency by one class.
    class TestImplementation : public WorkerThread<TestOperation, TestImplementation, TestRecord>
    {
        int workerId_;
        vector<TestNode>& model_;

    public:
        TestImplementation() = delete;

        // Required constructor.
        // Allow the template library to pass in the model
        // for each worker thread that is created.
        TestImplementation(int workerId, vector<TestNode>& model) :
            workerId_(workerId),
            model_(model)
        {
            
        }

        void Initialize(Log& log, Recorder<TestRecord>& record, 
            unsigned long long int ticksSinceEpoch, 
            ProcessCallback<TestOperation, TestRecord>& callback)
        {
            callback(TestOperation(1));
            log.Logger() << "Creating initial work for index " << 1 << " with tick " << 1 << '\n';
            log.Logit();
        }

        void Process(Log& log, Recorder<TestRecord>& record, 
        unsigned long long int ticksSinceEpoch, 
        typename vector<WorkItem<TestOperation>>::iterator begin, 
        typename vector<WorkItem<TestOperation>>::iterator end, 
        ProcessCallback<TestOperation, TestRecord>& callback)
        {
            // Do work here.
            if (begin == end) return;

            for (auto& work = begin; work != end; work++)
            {
                model_[work->Operator.Index].Data += workerId_;
                log.Logger() << "(" << work->Tick << ") Index " << work->Operator.Index << " set to " << model_[work->Operator.Index].Data << " in tick " << ticksSinceEpoch << '\n';
                log.Logit();
            }

            auto span = 5'000 / 7;
            auto index = 0LL;
            for (auto _ = 6; _--; index += span)
            {
                callback(TestOperation(index), 0);
                log.Logger() << "Creating work for index " << index << " with tick " << ticksSinceEpoch << '\n';
                log.Logit();
            }
            callback(TestOperation(4999), 0);
            log.Logger() << "Creating work for index " << 4999 << " with tick " << ticksSinceEpoch << '\n';
            log.Logit();
        }

        void Finalize(Log& log, Recorder<TestRecord>& record, int64_t milliseconds_since_epoch)
        {
        }
    };
}
