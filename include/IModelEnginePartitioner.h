#pragma once

#include <chrono>

namespace embeddedpenguins::modelengine
{
    using time_point = std::chrono::high_resolution_clock::time_point;

    //
    // IOC interface to allow for testing.
    //
    struct IModelEnginePartitioner
    {
        virtual ~IModelEnginePartitioner() = default;

        //
        // Merge work from the context of each worker
        // into a single work set, and extract the work
        // appropriate for this time slice, partitioning
        // work back into the context of each worker.
        //
        virtual unsigned long int Partition(unsigned long long int workCutoffTick) = 0;
    };
}
