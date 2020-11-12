#pragma once

#include <chrono>

#include "ModelEngineContextOp.h"
#include "IModelEngineWaiter.h"

namespace embeddedpenguins::modelengine
{
    using Clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::high_resolution_clock::time_point;
    using std::chrono::microseconds;

    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class ConstantTickWaiter : public IModelEngineWaiter
    {
        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& context_;
        ModelEngineContextOp<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE> contextOp_;
        time_point nextScheduledTick_;

    public:
        virtual unsigned long long int GetWorkCutoffTick() override { return context_.Iterations + 1; }

        ConstantTickWaiter(ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>& context) :
            context_(context),
            contextOp_(context),
            nextScheduledTick_(high_resolution_clock::now() + context_.EnginePeriod)
        {

        }

        virtual bool WaitForWorkOrQuit() override
        {
            //context_.Logger.Logger() << "ConstantTickWaiter::WaitForWorkOrQuit current tick time " << Log::FormatTime(nextScheduledTick_) << "\n";
            //context_.Logger.Logit();

            auto quit = contextOp_.WaitForWorkOrQuit(nextScheduledTick_);
            nextScheduledTick_ += context_.EnginePeriod;

            //context_.Logger.Logger() << "ConstantTickWaiter::WaitForWorkOrQuit woke with quit = " << quit << "\n";
            //context_.Logger.Logit();

            return quit;
        }
    };
}
