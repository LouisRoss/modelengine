#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>

#include "nlohmann/json.hpp"

#include "WorkerThread.h"
#include "WorkItem.h"
#include "ProcessCallback.h"
#include "Recorder.h"
#include "Log.h"

#include "ParticleCommon.h"
#include "ParticleSupport.h"
#include "ParticleOperation.h"
#include "ParticleNode.h"
#include "ParticleRecord.h"

//#define NOLOG
namespace embeddedpenguins::particle::infrastructure
{
    using std::cout;
    using std::vector;
    using std::unique;

    using nlohmann::json;

    using ::embeddedpenguins::modelengine::threads::WorkerThread;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;
    using ::embeddedpenguins::modelengine::Log;
    using ::embeddedpenguins::modelengine::Recorder;
    using ::embeddedpenguins::modelengine::WorkItem;

    //
    //
    class ParticleImplementation : public WorkerThread<ParticleOperation, ParticleImplementation, ParticleRecord>
    {
        int workerId_;
        vector<ParticleNode>& model_;
        const json& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };

    public:
        //
        // Recommended to not allow a default constructor.
        // Not required.
        //
        ParticleImplementation() = delete;

        //
        // Required constructor.
        // Allow the template library to pass in the model and configuration
        // to each worker thread that is created.
        //
        ParticleImplementation(int workerId, vector<ParticleNode>& model, const json& configuration) :
            workerId_(workerId),
            model_(model),
            configuration_(configuration)
        {
            // Override the dimension defaults if configured.
            auto dimensionElement = configuration_["Model"]["Dimensions"];
            if (dimensionElement.is_array())
            {
                auto dimensionArray = dimensionElement.get<vector<int>>();
                width_ = dimensionArray[0];
                height_ = dimensionArray[1];
            }

            maxIndex_ = width_ * height_;
        }

        //
        // Required Initialize method.  
        // One instance of this class on one thread will be called
        // here to initialize the model.
        // Do not use this method to initialize instances of this class.
        //
        void Initialize(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int tickNow, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
        }

        //
        // Required Process method.
        // Work items are partitioned in such a way that a single thread
        // may write to the model at the index in each work item in a
        // thread-safe manner.
        // Process the work items described by the iterators in whatever
        // manner is appropriate to the model.
        //
        void Process(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int tickNow, 
            typename vector<WorkItem<ParticleOperation>>::iterator begin, 
            typename vector<WorkItem<ParticleOperation>>::iterator end, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            if (end - begin == 0) return;

            // The work items tend to explode exponentially if we process duplicate work items.
            // Here we reduce the work load to include exactly one instance of each index.
            vector<WorkItem<ParticleOperation>> localWork(begin, end);
            auto endIt = unique(localWork.begin(), localWork.end(), 
                [](const WorkItem<ParticleOperation>& first, const WorkItem<ParticleOperation>& second)
                {
                    return first.Operator.Index == second.Operator.Index;
                });

            for (auto work = localWork.begin(); work != endIt; work++)
            {
                ProcessWorkItem(log, record, tickNow, work->Operator, callback);
            }
        }

        //
        // Required Finalize method.
        // One instance of this class on one thread will be called
        // here to clean up the model.
        // Do not use this method as a destructor for instances of this class.
        // Destructors are allowed -- use one instead.
        //
        void Finalize(Log& log, Recorder<ParticleRecord>& record, unsigned long long int tickNow)
        {
        }

    private:
        void ProcessWorkItem(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int tickNow, 
            const ParticleOperation& work, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            switch (work.Op)
            {
            case Operation::Propagate:
                ProcessPropagation(log, record, work.Index, callback);
                break;

            case Operation::Land:
                ProcessLanding(log, record, work.Index, work.VerticalVector, work.HorizontalVector, work.Mass, work.Speed, callback);
                break;

            default:
                break;
            }
        }

        //
        //
        void ProcessPropagation(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            auto& particleNode = model_[index];
 
            long long int tempIndex = (long long int)index + (particleNode.VerticalVector * width_) + particleNode.HorizontalVector;
            if (tempIndex >= maxIndex_) tempIndex = tempIndex - maxIndex_;
            if (tempIndex < 0) tempIndex = (long long int)maxIndex_ -1 /*+ tempIndex*/;

            auto nextIndex = (unsigned long long int)tempIndex;
            if (nextIndex >= maxIndex_)
            {
                cout << "Next landing index " << nextIndex << " (" << tempIndex << ") out of range!!\n";
                return;
            }
#ifndef NOLOG
            log.Logger() 
                << "Particle at node " << index 
                << " with vector [" << particleNode.VerticalVector << ',' << particleNode.HorizontalVector 
                << "] propagating to " << nextIndex << '\n';
            log.Logit();
#endif

            record.Record(ParticleRecord(ParticleRecordType::Land, index, particleNode));
            callback(ParticleOperation(nextIndex, particleNode.VerticalVector, particleNode.HorizontalVector, particleNode.Mass, particleNode.Speed));
            particleNode.Occupied = false;
        }

        void ProcessLanding(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            int verticalVector,
            int horizontalVector,
            int mass,
            int speed,
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            auto& particleNode = model_[index];

            particleNode.Occupied = true;
            particleNode.VerticalVector = verticalVector;
            particleNode.HorizontalVector = horizontalVector;
            particleNode.Mass = mass;
            particleNode.Speed = speed;

            record.Record(ParticleRecord(ParticleRecordType::Propagate, index, particleNode));
            callback(ParticleOperation(index), 5);
        }
    };
}
