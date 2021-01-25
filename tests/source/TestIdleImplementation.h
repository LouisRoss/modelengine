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
#include "TestModelCarrier.h"
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
        TestModelCarrier carrier_;
        const json& configuration_;
        bool firstRun_ { true };

    public:
        TestIdleImplementation() = delete;

        // Required constructor.
        // Allow the template library to pass in the model
        // for each worker thread that is created.
        TestIdleImplementation(int workerId, TestModelCarrier carrier, const json& configuration) :
            workerId_(workerId),
            carrier_(carrier),
            configuration_(configuration)
        {
            
        }

        void StreamNewInputWork(Log& log, Recorder<TestRecord>& record, 
            unsigned long long int ticksSinceEpoch, 
            ProcessCallback<TestOperation, TestRecord>& callback)
        {
            if (firstRun_) callback(TestOperation(1));

            firstRun_ = false;
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
                carrier_.Model[work->Operator.Index].Data += workerId_;
            }
        }
    };
}
