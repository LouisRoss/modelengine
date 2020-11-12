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
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class ConstantWidthPartitioner : public IModelEnginePartitioner
    {
        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& context_;

    public:
        ConstantWidthPartitioner(ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& context) :
            context_(context)
        {

        }

        virtual unsigned long int Partition(unsigned long long int _) override
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
            for (auto& sourceslice : context_.Workers)
            {
                for (auto& sourcework : sourceslice->GetContext().WorkForNextThread)
                {
                    for (auto& targetslice : context_.Workers)
                    {
                        WorkerContextOp<OPERATORTYPE, RECORDTYPE> targetContextOp(targetslice->GetContext());
                        if (targetContextOp.PushIfInRange(sourcework))
                            break;
                    }
                }

                sourceslice->GetContext().WorkForNextThread.clear();
            }
        }
    };
}
