#pragma once

#include "ModelEngineContextOp.h"
#include "IModelEngineWaiter.h"

namespace embeddedpenguins::modelengine
{
    //
    // Experimental waiter that will not run idle ticks.
    // Not currently in use.
    //
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE>
    class FirstWorkWaiter : public IModelEngineWaiter
    {
        ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE>& context_;
        ModelEngineContextOp<OPERATORTYPE, IMPLEMENTATIONTYPE> contextOp_;
        time_point workCutoffTime_;

    public:
        virtual time_point GetWorkCutoffTime() override { return workCutoffTime_; }

        FirstWorkWaiter(ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE>& context) :
            context_(context),
            contextOp_(context),
            workCutoffTime_(high_resolution_clock::now())
        {

        }
    };
}
