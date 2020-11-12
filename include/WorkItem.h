#pragma once

namespace embeddedpenguins::modelengine
{
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
