#pragma once

#include "ModelEngineContextOp.h"
#include "IModelEngineWaiter.h"

namespace embeddedpenguins::modelengine
{
    //
    // Experimental waiter that will not run idle ticks.
    // Not currently in use.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE>
    class FirstWorkWaiter : public IModelEngineWaiter
    {
        ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE>& context_;
        ModelEngineContextOp<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE> contextOp_;
        time_point workCutoffTime_;

    public:
        virtual time_point GetWorkCutoffTime() override { return workCutoffTime_; }

        FirstWorkWaiter(ModelEngineContext<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE>& context) :
            context_(context),
            contextOp_(context),
            workCutoffTime_(high_resolution_clock::now())
        {

        }

        virtual bool WaitForWorkOrQuit() override
        {
            auto quit = false;
            // TODO - this may be stale code that needs to be purged.
/*
            if (totalSourceWork_.empty())
            {
                //context_.Logger.Logger() << "Partition no work available, waiting for work or quit\n";
                //context_.Logger.Logit();
                quit = contextOp_.WaitForWorkOrQuit();
                //context_.Logger.Logger() << "Partition woke with quit = " << quit << "\n";
                //context_.Logger.Logit();
            }
            else
            {
                const auto& firstWork = std::min_element(
                    begin(totalSourceWork_), 
                    end(totalSourceWork_), 
                    [](const pair<time_point, OPERATORTYPE>& lhs, const pair<time_point, OPERATORTYPE>& rhs){ 
                        return lhs.first < rhs.first;
                    });
                wakeTime = firstWork->first;
                context_.Logger.Logger() << "Partition first work time " << Log::FormatTime(wakeTime) << "\n";
                context_.Logger.Logit();

                quit = context_.WaitForWorkOrQuit(firstWork->first);
                context_.Logger.Logger() << "Partition woke with quit = " << quit << "\n";
                context_.Logger.Logit();
            }
*/
            return quit;
        }
    };
}
