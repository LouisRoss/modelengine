#pragma once

namespace embeddedpenguins::modelengine
{
    //
    // Work to be done by the worker threads is abstracted as
    // a collection of instances of this class.
    //
    template<class OPERATORTYPE>
    struct WorkItem
    {
        unsigned long long int Tick;
        OPERATORTYPE Operator;

        bool operator<(const WorkItem<OPERATORTYPE>& other)
        {
            return Tick < other.Tick;
        }
    };
}
