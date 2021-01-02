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
        // Do a timed wait for the quit signal.  Return true
        // if we need to quit, false if the timeout occurred.
        //
        virtual bool WaitForWorkOrQuit() = 0;
    };
}
