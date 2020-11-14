#pragma once

#include "ProcessCallback.h"

namespace embeddedpenguins::modelengine::sdk
{
    using embeddedpenguins::modelengine::threads::ProcessCallback;

    //
    // All model initializers must implement this interface, to allow
    // interchangability between initializers.
    //
    template<class OPERATORTYPE, class RECORDTYPE>
    class IModelInitializer
    {
    public:
        virtual ~IModelInitializer() = default;

        //
        // Called before the model is run, this required method must 
        // initialize any state needed for a specific use case.
        //
        virtual void Initialize() = 0;

        //
        // Called after the model is running, this optional method
        // may dynamically inject work into the modelengine as operators.
        //
        virtual void InjectSignal(ProcessCallback<OPERATORTYPE, RECORDTYPE>& callback) { }
    };
}
