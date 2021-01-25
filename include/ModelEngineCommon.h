#pragma once

#include <chrono>

namespace embeddedpenguins
{
    namespace modelengine::threads
    {
        enum class WorkCode
        {
            Run,
            Quit,
            Scan
        };
    }

    namespace modelengine
    {
        using std::chrono::duration_cast;
        using std::chrono::high_resolution_clock;
        using std::chrono::microseconds;

        enum class PartitionPolicy
        {
            ConstantWidth,
            AdaptiveWidth
        };
    }
}
