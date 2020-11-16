#pragma once

#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>

#include "nlohmann/json.hpp"

#include "WorkerThread.h"
#include "WorkItem.h"
#include "ProcessCallback.h"
#include "Recorder.h"
#include "TestOperation.h"
#include "TestNode.h"
#include "TestRecord.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using std::vector;
    using std::pair;
    using std::for_each;
    using std::cout;
    using std::chrono::milliseconds;
    using time_point = std::chrono::high_resolution_clock::time_point;

    using nlohmann::json;

    using ::embeddedpenguins::modelengine::threads::WorkerThread;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;
    using ::embeddedpenguins::modelengine::Log;
    using ::embeddedpenguins::modelengine::Recorder;
    using ::embeddedpenguins::modelengine::WorkItem;

    // Note: the callback should be allowed to be declared something like
    // void (*callback)(const SpikingOperation&)
    // but I could not get that to work.  As a workaround,
    // I have explicitly referenced the functor class used
    // by the template library (ProcessCallback<SpikingOperation>).
    // It would be great to get this to work so I could reduce the
    // dependency by one class.
    class TestIdleImplementation : public WorkerThread<TestOperation, TestIdleImplementation, TestRecord>
    {
        int workerId_;
        vector<TestNode>& model_;
        const json& configuration_;

    public:
        TestIdleImplementation() = delete;

        // Required constructor.
        // Allow the template library to pass in the model
        // for each worker thread that is created.
        TestIdleImplementation(int workerId, vector<TestNode>& model, const json& configuration) :
            workerId_(workerId),
            model_(model),
            configuration_(configuration)
        {
            
        }

        void Initialize(Log& log, Recorder<TestRecord>& record, 
            unsigned long long int ticksSinceEpoch, 
            ProcessCallback<TestOperation, TestRecord>& callback)
        {
            callback(TestOperation(1));
        }

        // Process but add no new work.  After the first single work item, the whole model will be idle.
        void Process(Log& log, Recorder<TestRecord>& record, 
            unsigned long long int ticksSinceEpoch, 
            typename vector<WorkItem<TestOperation>>::iterator begin, 
            typename vector<WorkItem<TestOperation>>::iterator end, 
            ProcessCallback<TestOperation, TestRecord>& callback)
        {
            // Do work here.
            for (auto& work = begin; work != end; work++)
            {
                model_[work->Operator.Index].Data += workerId_;
            }
        }

        void Finalize(Log& log, Recorder<TestRecord>& record, int64_t milliseconds_since_epoch)
        {
        }
    };
}
