#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
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

#define NOLOG
namespace embeddedpenguins::particle::infrastructure
{
    using std::cout;
    using std::string;
    using std::memset;
    using std::memcpy;
    using std::vector;
    using std::multimap;
    using std::unique;
    using std::make_pair;
    using std::make_tuple;
    using std::tuple;

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
#if false
            // Put the Land operations into a multimap sorted by index.
            multimap<unsigned long long int, ParticleOperation> clusteredWork {};
            for (auto work = begin; work != end; work++)
            {
                if (work->Operator.Op == Operation::Land)
                    clusteredWork.insert(make_pair(work->Operator.Index, work->Operator));
            }

            // Handle any collisions of Land operations by finding consecutive entries with the same index.
            if (!clusteredWork.empty())
            {
                auto almostEnd = clusteredWork.end();
                almostEnd--;
                for (auto work = clusteredWork.begin(); work != clusteredWork.end(); work++)
                {
                    auto clustering { true };
                    auto clusterFound { false };
                    unsigned long long int clusterIndex {};
                    vector<ParticleOperation> cluster {};
                    while (work != almostEnd && clustering)
                    {
                        auto nextWork = work;
                        nextWork++;
                        if (work->first == nextWork->first)
                        {
                            clusterIndex = work->first;
                            clusterFound = true;
                            work++;

                            cluster.push_back(work->second);
                        }
                        else
                        {
                            if (clusterFound) cluster.push_back(work->second);
                            clustering = false;
                        }
                    }

                    if (clusterFound)
                    {
                        ProcessCollision(log, record, clusterIndex, cluster, callback);
                    }
                    else
                    {
                        ProcessWorkItem(log, record, tickNow, work->second, callback);
                    }
                }
            }
#endif
            // Handle the remaining non-Land operations normally.
            for (auto work = begin; work != end; work++)
            {
                //if (work->Operator.Op != Operation::Land)
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
            string particleName(work.Name);
            switch (work.Op)
            {
            case Operation::Propagate:
                ProcessPropagation(log, record, particleName, work.Index, callback);
                break;

            case Operation::Land:
                ProcessLanding(log, record, work.Index, particleName, work.VerticalVector, work.HorizontalVector, work.Mass, work.Speed, callback);
                break;

            default:
                break;
            }
        }

        //
        //
        void ProcessCollision(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            const vector<ParticleOperation>& cluster,
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            log.Logger() << "Collision of " << cluster.size() << " nodes at index " << index << '\n';
            log.Logit();
            for (const auto& op : cluster)
            {
                auto verticalVector = -1 * op.VerticalVector;
                auto horizontalVector = -1 * op.HorizontalVector;

                auto [nextIndex, nextVerticalVector, nextHorizontalVector] = NewPositionAndVelocity(log, record, index, verticalVector, horizontalVector, callback);

                log.Logger() << op.Name << "  Collision node propagated from index " << index << " to index " << nextIndex << " using vector (" << nextVerticalVector << ',' << nextHorizontalVector << ")\n";
                log.Logit();
                callback(ParticleOperation(nextIndex, op.Name, nextVerticalVector, nextHorizontalVector, op.Mass, op.Speed));
            }
        }

        //
        //
        void ProcessPropagation(Log& log, Recorder<ParticleRecord>& record, 
            const string& name,
            unsigned long long int index, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            auto& particleNode = model_[index];

            auto [nextIndex, nextVerticalVector, nextHorizontalVector] = NewPositionAndVelocity(log, record, index, particleNode.VerticalVector, particleNode.HorizontalVector, callback);

            record.Record(ParticleRecord(name, ParticleRecordType::Land, nextIndex, particleNode));
            callback(ParticleOperation(nextIndex, name, nextVerticalVector, nextHorizontalVector, particleNode.Mass, particleNode.Speed));

            log.Logger() << '<' << name << "> " << "Particle propagating from (" << particleNode.Name << ") index " << index << "\n";
            log.Logit();
            memset(particleNode.Name, '\0', sizeof(particleNode.Name));
            particleNode.Occupied = false;
        }

        //
        //
        tuple<unsigned long long int, int, int> NewPositionAndVelocity(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            int verticalVector,
            int horizontalVector,
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            // NOTE - stay away from mixed signed/unsigned comparisons by casting everything signed.
            int width = (int)width_;
            int height = (int)height_;
            long long int currentIndex = (long long int)index;

            int nextVerticalPosition = currentIndex / width;
            int nextVerticalVector = verticalVector;
            nextVerticalPosition += nextVerticalVector;
            if (nextVerticalPosition >= height)
            {
                nextVerticalPosition = height - (nextVerticalPosition - height) - 1;
                nextVerticalVector *= -1;
            }
            else if (nextVerticalPosition < 0)
            {
                nextVerticalPosition = 0 - nextVerticalPosition;
                nextVerticalVector *= -1;
            }

            int nextHorizontalPosition = currentIndex % width;
            int nextHorizontalVector = horizontalVector;
            nextHorizontalPosition += nextHorizontalVector;
            if (nextHorizontalPosition >= width)
            {
                nextHorizontalPosition = width - (nextHorizontalPosition - width) - 1;
                nextHorizontalVector *= -1;
            }
            else if (nextHorizontalPosition < 0)
            {
                nextHorizontalPosition = 0 - nextHorizontalPosition;
                nextHorizontalVector *= -1;
            }

            auto nextIndex = (unsigned long long int)nextVerticalPosition * width_ + (unsigned long long int)nextHorizontalPosition;

#ifndef NOLOG
            log.Logger() 
                << "Particle at node " << index 
                << " with vector [" << verticalVector << ',' << horizontalVector 
                << "] propagating to " << nextIndex << '\n';
            log.Logit();
#endif
            return make_tuple(nextIndex, nextVerticalVector, nextHorizontalVector);
        }

        void ProcessLanding(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            const string& name,
            int verticalVector,
            int horizontalVector,
            int mass,
            int speed,
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            auto& particleNode = model_[index];

            if (particleNode.Occupied)
            {
                verticalVector = -1 * verticalVector;
                horizontalVector = -1 * horizontalVector;

                auto [nextIndex, nextVerticalVector, nextHorizontalVector] = NewPositionAndVelocity(log, record, index, verticalVector, horizontalVector, callback);

                log.Logger() << '<' << name << "> " << "Particle collision at index " << index << " already occupied by <" << particleNode.Name << ">\n";
                log.Logit();
                callback(ParticleOperation(nextIndex, name, nextVerticalVector, nextHorizontalVector, mass, speed));
                return;
            }

            memset(particleNode.Name, '\0', sizeof(particleNode.Name));
            memcpy(particleNode.Name, name.c_str(), (name.length() < sizeof(particleNode.Name) - 1 ? name.length() : sizeof(particleNode.Name) - 1));
            particleNode.Occupied = true;
            particleNode.VerticalVector = verticalVector;
            particleNode.HorizontalVector = horizontalVector;
            particleNode.Mass = mass;
            particleNode.Speed = speed;

            log.Logger() << '<' << particleNode.Name << "> " << "Particle landed at index " << index << "\n";
            log.Logit();
            record.Record(ParticleRecord(name, ParticleRecordType::Propagate, index, particleNode));
            callback(ParticleOperation(index, name), 10 - particleNode.Speed);
        }
    };
}
