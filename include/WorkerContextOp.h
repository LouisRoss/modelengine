#pragma once

#include <vector>

#include "WorkerContext.h"
#include "WorkItem.h"

namespace embeddedpenguins::modelengine::threads
{
    using std::vector;

    using embeddedpenguins::modelengine::WorkItem;

    //
    // Separate the executable code from the context carrier object so that the
    // context carrier object may be passed around without exposing methods.
    // 
    template<class OPERATORTYPE, class RECORDTYPE>
    class WorkerContextOp
    {
        WorkerContext<OPERATORTYPE, RECORDTYPE>& context_;

    public:
        WorkerContextOp(WorkerContext<OPERATORTYPE, RECORDTYPE>& context) :
            context_(context)
        {

        }

        void CaptureWorkForThread(
            typename vector<WorkItem<OPERATORTYPE>>::iterator segmentBegin, 
            typename vector<WorkItem<OPERATORTYPE>>::iterator segmentEnd)
        {
            context_.WorkForThread.clear();
            context_.WorkForThread.insert(std::end(context_.WorkForThread), segmentBegin, segmentEnd);
        }

        bool PushIfInRange(WorkItem<OPERATORTYPE>& work)
        {
            auto inRange = !(work.Operator.Index < context_.RangeBegin) && (work.Operator.Index < context_.RangeEnd);
            if (!inRange) return false;

            context_.WorkForThread.push_back(work);
            return true;
        }
    };
}
