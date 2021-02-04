#pragma once

#include "IModelEnginePartitioner.h"
#include "ModelEngineCommon.h"
#include "ModelEngineContext.h"
#include "Worker.h"
#include "WorkerContextOp.h"

namespace embeddedpenguins::modelengine
{
    using embeddedpenguins::modelengine::threads::Worker;
    using embeddedpenguins::modelengine::threads::WorkerContextOp;

    //
    //  ConstantWidthPartitioner.  Always partition the full model into equal-size
    // partitions, or stripes.  In sparsely-active models, this may result in one
    // or more workers going idle during many scans.
    //
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELCARRIERTYPE, class RECORDTYPE>
    class ConstantWidthPartitioner : public IModelEnginePartitioner
    {
        ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& context_;

    public:
        ConstantWidthPartitioner(ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELCARRIERTYPE, RECORDTYPE>& context) :
            context_(context)
        {

        }

        virtual void ConcurrentPartitionStep() override
        {

        }

        virtual unsigned long int SingleThreadPartitionStep() override
        {
            auto totalWork = AccumulateWorkfromWorkers();

            ++context_.Iterations;

            DistributeWorkToWorkers();

            return totalWork;
        }

    protected:
        unsigned long int AccumulateWorkfromWorkers()
        {
            unsigned long int totalWork {};
            for (auto& targetslice : context_.Workers)
            {
                auto workForThread = targetslice->GetContext().WorkForThread.size();
                totalWork += workForThread;
                context_.TotalWork += workForThread;
                targetslice->GetContext().WorkForThread.clear();
            }

            return totalWork;
        }

        void DistributeWorkToWorkers()
        {
            for (auto& sourceWorker : context_.Workers)
            {
                auto& workForFutureTicks = (sourceWorker->GetContext().CurrentBuffer == CurrentBufferType::Buffer2Current) ? 
                        sourceWorker->GetContext().WorkForFutureTicks1 : 
                        sourceWorker->GetContext().WorkForFutureTicks2;
                for (auto& sourcework : workForFutureTicks)
                //for (auto& sourcework : sourceWorker->GetContext().WorkForNextThread)
                {
                    for (auto& targetslice : context_.Workers)
                    {
                        WorkerContextOp<OPERATORTYPE, RECORDTYPE> targetContextOp(targetslice->GetContext());
                        if (targetContextOp.PushIfInRange(sourcework))
                            break;
                    }
                }

                workForFutureTicks.clear();
            }
        }
    };
}
