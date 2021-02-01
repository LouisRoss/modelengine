#pragma once

#include "ModelEngineCommon.h"
#include "IModelInitializer.h"

namespace embeddedpenguins::modelengine::sdk
{
    using embeddedpenguins::modelengine::ConfigurationUtilities;

    //
    // Base class for all model initializers.  Since an initializer
    // must reference the configuration, a reference to it is captured here.
    //
    template<class OPERATORTYPE, class MODELHELPERTYPE, class RECORDTYPE>
    class ModelInitializer : public IModelInitializer<OPERATORTYPE, RECORDTYPE>
    {
    protected:
        ConfigurationUtilities& configuration_;
        MODELHELPERTYPE helper_;

    public:
        ModelInitializer(ConfigurationUtilities& configuration, MODELHELPERTYPE helper) :
            configuration_(configuration),
            helper_(helper)
        {
        }
    };
}
