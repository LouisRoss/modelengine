#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <execution>

#include "IModelEnginePartitioner.h"
#include "ModelEngineCommon.h"
#include "ModelEngineContext.h"
#include "WorkerContextOp.h"
#include "Worker.h"
#include "WorkItem.h"

namespace embeddedpenguins::modelengine
{
    using std::vector;
    using std::pair;
    using std::begin;
    using std::end;
    using std::cout;
    using std::chrono::milliseconds;
    using Clock = std::chrono::high_resolution_clock;
    using embeddedpenguins::modelengine::threads::Worker;
    using embeddedpenguins::modelengine::threads::WorkerContextOp;

    //
    // AdaptiveWidthPartitioner.  For each scan, partition work to workers,
    // by partitioning the work for the scan into contiguous memory 'stripes'.
    // Each worker runs its own thread, and is allowed to write into model memory 
    // within its 'stripe', but not to memory in other 'stripes'.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class AdaptiveWidthPartitioner : public IModelEnginePartitioner
    {
        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& context_;

        // Expose some internal state to derived classes to allow for testing.
    protected:
        vector<WorkItem<OPERATORTYPE>> totalSourceWork_ {};

    public:
        AdaptiveWidthPartitioner(ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& context) :
            context_(context)
        {
        }

        virtual unsigned long int Partition(unsigned long long int workCutoffTick) override
        {
#ifndef NOLOG
            context_.Logger.Logger() << "Starting Partition\n";
            context_.Logger.Logit();
#endif

            AccumulateWorkFromAllWorkers();
            auto cutoffPoint = FindCutoffPoint(workCutoffTick);

            vector<WorkItem<OPERATORTYPE>> workForTimeSlice(begin(totalSourceWork_), cutoffPoint);
            totalSourceWork_.erase(begin(totalSourceWork_), cutoffPoint);
#ifndef NOLOG
            if (!workForTimeSlice.empty())
            {
                context_.Logger.Logger() << "Partitioning found " << workForTimeSlice.size() << " work items for tick " << workCutoffTick << ", leaving " << totalSourceWork_.size() << " for future ticks\n";
                context_.Logger.Logit();
            }
#endif

            auto totalWork = workForTimeSlice.size();
            context_.TotalWork += totalWork;
            ++context_.Iterations;

            CaptureWorkForEachThread(workForTimeSlice);

            return totalWork;
        }

        // Expose some internal methods to derived classes to allow for testing.
    protected:
        void AccumulateWorkFromAllWorkers()
        {
            for (auto& sourceWorker : context_.Workers)
            {
                auto& sourceWork = sourceWorker->GetContext().WorkForNextThread;
                totalSourceWork_.insert(
                    end(totalSourceWork_), 
                    begin(sourceWork), 
                    end(sourceWork));
                sourceWork.clear();
            }

            auto& externalSourceWork = context_.ExternalWorkSource.WorkForNextThread;
            totalSourceWork_.insert(
                end(totalSourceWork_), 
                begin(externalSourceWork), 
                end(externalSourceWork));
            externalSourceWork.clear();
        }

        typename vector<WorkItem<OPERATORTYPE>>::iterator FindCutoffPoint(unsigned long long int workCutoffTick)
        {
            auto cutoffPoint = std::partition(
                totalSourceWork_.begin(), 
                totalSourceWork_.end(), 
                [workCutoffTick](const WorkItem<OPERATORTYPE>& work){
                    return work.Tick < workCutoffTick;
            });

            return cutoffPoint;
        }

        void CaptureWorkForEachThread(vector<WorkItem<OPERATORTYPE>>& workForTimeSlice)
        {
#ifndef NOLOG
            context_.Logger.Logger() << "CaptureWorkForEachThread starting sort\n";
            context_.Logger.Logit();
#endif
            std::sort(
                std::execution::par,
                begin(workForTimeSlice), 
                end(workForTimeSlice) , 
                [](const WorkItem<OPERATORTYPE>& lhs, const WorkItem<OPERATORTYPE>& rhs){ 
                    return lhs.Operator.Index < rhs.Operator.Index;
            });

#ifndef NOLOG
            context_.Logger.Logger() << "CaptureWorkForEachThread allocating segments to worker threads\n";
            context_.Logger.Logit();
#endif

            auto segmentSize = workForTimeSlice.size() / context_.WorkerCount;
            if (segmentSize < 1) segmentSize = 1;

            auto segmentBegin = begin(workForTimeSlice);

            for (auto workerIndex = 0; workerIndex < context_.Workers.size(); workerIndex++)
            {
                auto segmentEnd = FindSegmentEnd(workForTimeSlice, segmentSize, segmentBegin, workerIndex);

                auto& targetWorker = context_.Workers[workerIndex];
                WorkerContextOp<OPERATORTYPE, RECORDTYPE> contextOp(targetWorker->GetContext());
                contextOp.CaptureWorkForThread(segmentBegin, segmentEnd);

                segmentBegin = segmentEnd;
            }
        }

        typename vector<WorkItem<OPERATORTYPE>>::iterator FindSegmentEnd(
            vector<WorkItem<OPERATORTYPE>>& workForTimeSlice, 
            int segmentSize, 
            typename vector<WorkItem<OPERATORTYPE>>::iterator segmentBegin,
            int workerIndex)
        {
            auto segmentEnd = end(workForTimeSlice) - segmentBegin <= segmentSize ? end(workForTimeSlice) : segmentBegin + segmentSize;

            if (segmentEnd != end(workForTimeSlice) && workerIndex < context_.Workers.size() - 1)
            {
                auto previousIndex = (segmentEnd - 1)->Operator.Index;
                while (segmentEnd != end(workForTimeSlice) && segmentEnd->Operator.Index == previousIndex)
                    segmentEnd++;
            }
            else
            {
                segmentEnd = end(workForTimeSlice);
            }

            return segmentEnd;
        }
    };
}
