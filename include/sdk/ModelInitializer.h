#pragma once

#include "IModelInitializer.h"
#include "nlohmann/json.hpp"

namespace embeddedpenguins::modelengine::sdk
{
    using nlohmann::json;

    //
    // Base class for all model initializers.  Since an initializer
    // must reference the configuration, a reference to it is captured here.
    //
    template<class OPERATORTYPE, class MODELHELPERTYPE, class RECORDTYPE>
    class ModelInitializer : public IModelInitializer<OPERATORTYPE, RECORDTYPE>
    {
    protected:
        json& configuration_;
        MODELHELPERTYPE helper_;

    public:
        ModelInitializer(json& configuration, MODELHELPERTYPE helper) :
            configuration_(configuration),
            helper_(helper)
        {
        }
    };
}
