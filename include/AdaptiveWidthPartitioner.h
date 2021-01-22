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
    using embeddedpenguins::modelengine::threads::CurrentBufferType;

    //
    // AdaptiveWidthPartitioner.  For each scan, partition work to workers,
    // by partitioning the work for the scan into contiguous memory 'stripes'.
    // Each worker runs its own thread, and is allowed to write into model memory 
    // within its 'stripe', but not to memory in other 'stripes'.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELCARRIERTYPE, class RECORDTYPE>
    class AdaptiveWidthPartitioner : public IModelEnginePartitioner
    {
        // Expose some internal state to derived classes to allow for testing.
    protected:
        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& context_;
        vector<WorkItem<OPERATORTYPE>> totalSourceWork_ {};
        vector<WorkItem<OPERATORTYPE>> workForNextTick_ {};

    public:
        AdaptiveWidthPartitioner(ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& context) :
            context_(context)
        {
        }

        //
        // The controlling thread will start the worker threads with work for this tick,
        // then call this to allow the partitioner to do anything that is possible while
        // the worker threads are running.
        //
        virtual void ConcurrentPartitionStep() override
        {
#ifndef NOLOG
            context_.Logger.Logger() << "Tick " << context_.Iterations << ": Accumulating future work from all workers and Splitting out work for next tick\n";
            context_.Logger.Logit();
#endif

            AccumulateFutureWorkFromAllWorkers();
            SplitOutWorkForNextTick();
        }

        //
        // After the worker threads are done, and only the controlling thread is running,
        // it will call here to allow partitioning that reqires access to shared memory.
        //
        virtual unsigned long int SingleThreadPartitionStep() override
        {
#ifndef NOLOG
            context_.Logger.Logger() << "Tick " << context_.Iterations << ": Accumulating next tick work from all workers and partitioning to all workers\n";
            context_.Logger.Logit();
#endif

            AccumulateWorkForNextTickFromAllWorkers();
            return PartitionWorkForNextTickToAllWorkers();
        }

        // Expose some internal methods to derived classes to allow for testing.
    protected:
        //
        // Accumulate all future work (work that is scheduled later than the upcoming tick)
        // from all worker threads, as it was created in the previous tick, into the work backlog.  
        // In the current tick, the worker threads are using the current buffer, 
        // so the other buffer is free for us to access read/write.
        // NOTE: This may run concurrently with the worker threads.
        //
        void AccumulateFutureWorkFromAllWorkers()
        {
            for (auto& sourceWorker : context_.Workers)
            {
                // As the partitioner, use the opposite buffer from the current one.
                auto& sourceWork = 
                    (sourceWorker->GetContext().CurrentBuffer == CurrentBufferType::Buffer2Current) ? 
                        sourceWorker->GetContext().WorkForFutureTicks1 : 
                        sourceWorker->GetContext().WorkForFutureTicks2;
                totalSourceWork_.insert(
                    end(totalSourceWork_), 
                    begin(sourceWork), 
                    end(sourceWork));
                sourceWork.clear();
            }

            auto& externalSourceWork = 
                (context_.ExternalWorkSource.CurrentBuffer == CurrentBufferType::Buffer2Current) ? 
                    context_.ExternalWorkSource.WorkForFutureTicks1 : 
                    context_.ExternalWorkSource.WorkForFutureTicks2;
            totalSourceWork_.insert(
                end(totalSourceWork_), 
                begin(externalSourceWork), 
                end(externalSourceWork));
            externalSourceWork.clear();
        }

        //
        // The work backlog has work from all previous ticks, scheduled for a specific tick.
        // Collect any work scheduled for the next tick, and split it out from the work backlog.
        // NOTE: This may run concurrently with the worker threads.
        //
        void SplitOutWorkForNextTick()
        {
            auto cutoffPoint = FindCutoffPoint();

            workForNextTick_.clear();
            workForNextTick_.insert(
                end(workForNextTick_), 
                begin(totalSourceWork_), 
                cutoffPoint);
            totalSourceWork_.erase(begin(totalSourceWork_), cutoffPoint);
        }

        //
        // Accumulate all next-tick work from all worker threads, as it was created in the 
        // just-completed current tick.  There is only one buffer per worker thread for next-tick
        // work.
        // NOTE: This MUST NOT run concurrently with the worker threads.
        //
        void AccumulateWorkForNextTickFromAllWorkers()
        {
            for (auto& worker : context_.Workers)
            {
                auto& workForTick1 = worker->GetContext().WorkForTick1;
                workForNextTick_.insert(
                    end(workForNextTick_), 
                    begin(workForTick1), 
                    end(workForTick1));

                workForTick1.clear();
            }

            auto& externalworkForTick1 = context_.ExternalWorkSource.WorkForTick1;
            workForNextTick_.insert(
                end(workForNextTick_), 
                begin(externalworkForTick1), 
                end(externalworkForTick1));

            externalworkForTick1.clear();
#ifndef NOLOG
            if (!workForNextTick_.empty())
            {
                context_.Logger.Logger() << "Partitioning found " << workForNextTick_.size() << " work items for tick " << context_.Iterations + 1 << ", leaving " << totalSourceWork_.size() << " for future ticks\n";
                context_.Logger.Logit();
            }
#endif
        }

        //
        // The work for next tick contains work from all previous ticks split
        // out from the work backlog, plus any work newly-generated by the workers
        // in the current tick.
        // Sort by index and partition to workers for next tick.
        // NOTE: This MUST NOT run concurrently with the worker threads.
        //
        unsigned long int PartitionWorkForNextTickToAllWorkers()
        {
            auto totalWork = workForNextTick_.size();
            context_.TotalWork += totalWork;

#ifndef NOLOG
            context_.Logger.Logger() << "PartitionWorkForNextTickToAllWorkers starting sort\n";
            context_.Logger.Logit();
#endif
            std::sort(
                std::execution::par,
                begin(workForNextTick_), 
                end(workForNextTick_) , 
                [](const WorkItem<OPERATORTYPE>& lhs, const WorkItem<OPERATORTYPE>& rhs){ 
                    return lhs.Operator.Index < rhs.Operator.Index;
            });

#ifndef NOLOG
            context_.Logger.Logger() << "PartitionWorkForNextTickToAllWorkers allocating segments to worker threads\n";
            context_.Logger.Logit();
#endif

            auto segmentSize = workForNextTick_.size() / context_.WorkerCount;
            if (segmentSize < 1) segmentSize = 1;

            auto segmentBegin = begin(workForNextTick_);

            for (auto workerIndex = 0; workerIndex < context_.Workers.size(); workerIndex++)
            {
                auto segmentEnd = FindSegmentEnd(workForNextTick_, segmentSize, segmentBegin, workerIndex);

                auto& targetWorker = context_.Workers[workerIndex];
                WorkerContextOp<OPERATORTYPE, RECORDTYPE> contextOp(targetWorker->GetContext());
                contextOp.CaptureWorkForThread(segmentBegin, segmentEnd);

                segmentBegin = segmentEnd;
            }

            return totalWork;
        }

        typename vector<WorkItem<OPERATORTYPE>>::iterator FindCutoffPoint()
        {
            auto workCutoffTick = context_.Iterations + 1;
            auto cutoffPoint = std::partition(
                totalSourceWork_.begin(), 
                totalSourceWork_.end(), 
                [workCutoffTick](const WorkItem<OPERATORTYPE>& work){
                    return work.Tick < workCutoffTick;
            });

            return cutoffPoint;
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
