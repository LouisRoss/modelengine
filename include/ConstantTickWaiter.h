#pragma once

#include <chrono>

#include "ModelEngineContextOp.h"
#include "IModelEngineWaiter.h"

namespace embeddedpenguins::modelengine
{
    using std::chrono::high_resolution_clock;
    using time_point = std::chrono::high_resolution_clock::time_point;
    using std::chrono::microseconds;

    //
    // This waiter is typically used for models, and always waits until the
    // next scheduled tick in time.  If a model execution runs longer than
    // a tick time, this waiter will schedule the next run in a attempt to 
    // catch up.
    //
    template<class OPERATORTYPE, class IMPLEMENTATIONTYPE, class MODELHELPERTYPE, class RECORDTYPE>
    class ConstantTickWaiter : public IModelEngineWaiter
    {
        ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>& context_;
        ModelEngineContextOp<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE> contextOp_;
        time_point nextScheduledTick_;
        bool waitForFirstTick { true };

    public:
        ConstantTickWaiter(ModelEngineContext<OPERATORTYPE, IMPLEMENTATIONTYPE, MODELHELPERTYPE, RECORDTYPE>& context) :
            context_(context),
            contextOp_(context),
            nextScheduledTick_(high_resolution_clock::now() + context_.EnginePeriod)
        {

        }

        virtual bool WaitForWorkOrQuit() override
        {
            if (waitForFirstTick)
            {
                nextScheduledTick_ = high_resolution_clock::now() + context_.EnginePeriod;
                waitForFirstTick = false;
            }
            
            ///context_.Logger.Logger() << "ConstantTickWaiter::WaitForWorkOrQuit current tick time " << Log::FormatTime(nextScheduledTick_) << "\n";
            //context_.Logger.Logit();

            auto quit = contextOp_.WaitForWorkOrQuit(nextScheduledTick_);
            nextScheduledTick_ += context_.EnginePeriod;

            //context_.Logger.Logger() << "ConstantTickWaiter::WaitForWorkOrQuit woke with quit = " << quit << "\n";
            //context_.Logger.Logit();

            return quit;
        }
    };
}
