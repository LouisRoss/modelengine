#pragma once

#//include <chrono>

namespace embeddedpenguins::modelengine
{
    //using time_point = std::chrono::high_resolution_clock::time_point;

    //
    // IOC interface to allow for testing.
    //
    struct IModelEngineWaiter
    {
        virtual ~IModelEngineWaiter() = default;

        //
        // Get the tick to use when partitioning the work backlog.
        // Work older than this cutoff gets done now, newer work waits.
        //
        virtual unsigned long long int GetWorkCutoffTick() = 0;

        //
        // Do a timed wait for the quit signal.  Return true
        // if we need to quit, false if the timeout occurred.
        //
        virtual bool WaitForWorkOrQuit() = 0;
    };
}
