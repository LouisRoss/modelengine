#pragma once

#include "IModelInitializer.h"

namespace embeddedpenguins::modelengine::sdk
{
    using embeddedpenguins::core::neuron::model::IModelInitializer;

    //
    // Base class for all model initializers.  Since an initializer
    // must reference the configuration, a reference to it is captured here.
    //
    template<class MODELHELPERTYPE>
    class ModelInitializer : public IModelInitializer<MODELHELPERTYPE>
    {
    protected:
        MODELHELPERTYPE helper_;

    public:
        ModelInitializer(MODELHELPERTYPE helper) :
            helper_(helper)
        {
        }

    public:
        // IModelInitializer interface, must be implemented, but not used if not a proxy class.
        virtual void CreateProxy(MODELHELPERTYPE helper) { }
    };
}
