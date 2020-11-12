#pragma once

#include <vector>
#include "IModelInitializer.h"
#include "nlohmann/json.hpp"

namespace embeddedpenguins::modelengine::sdk
{
    using std::vector;
    using nlohmann::json;

    //
    // Base class for all model initializers.  Since an initializer
    // must reference the model, this common behavior is captured here.
    //
    template<class NODETYPE, class OPERATORTYPE, class IMPLEMENTATIONTYPE, class RECORDTYPE>
    class ModelInitializer : public IModelInitializer<NODETYPE, OPERATORTYPE, IMPLEMENTATIONTYPE, RECORDTYPE>
    {
    protected:
        vector<NODETYPE>& model_;
        json& configuration_;

    public:
        ModelInitializer(vector<NODETYPE>& model, json& configuration) :
            model_(model),
            configuration_(configuration)
        {
        }
    };
}
