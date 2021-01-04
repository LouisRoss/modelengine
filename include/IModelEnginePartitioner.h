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
        // It is assumed that some preparatory partitioning work
        // can be started while the worker threads are running.
        // Do whatever multithreaded partitioning is possible here.
        //
        virtual void ConcurrentPartitionStep() = 0;

        //
        // Finish the partitioning work after the worker threads
        // have completed.  The full context of each worker is now
        // available read/write.
        // Merge work from the context of each worker
        // into a single work set, and extract the work
        // appropriate for this time slice, partitioning
        // work back into the context of each worker.
        //
        virtual unsigned long int SingleThreadPartitionStep() = 0;
    };
}
