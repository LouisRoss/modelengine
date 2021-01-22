#pragma once

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace embeddedpenguins::neuron::infrastructure::persistence
{
    using std::string;
    using std::vector;
    using nlohmann::json;

    //
    // TODO: Use this interface for persistence of the current state of a model.
    //
    template<class NODETYPE>
    class IModelPersister
    {
    public:
        virtual ~IModelPersister() = default;

        virtual bool LoadConfiguration() = 0;
        virtual bool ReadModel(vector<NODETYPE>& model, json& configuration) = 0;
    };
}
