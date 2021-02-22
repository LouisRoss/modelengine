#pragma once

#include <string>
#include <vector>

#include "ConfigurationRepository.h"

namespace embeddedpenguins::neuron::infrastructure::persistence
{
    using std::string;
    using std::vector;

    using embeddedpenguins::core::neuron::model::ConfigurationRepository;

    //
    // TODO: Use this interface for persistence of the current state of a model.
    //
    template<class MODELHELPERTYPE>
    class IModelPersister
    {
    public:
        virtual ~IModelPersister() = default;

        virtual bool LoadConfiguration() = 0;
        virtual bool ReadModel(MODELHELPERTYPE& helper, ConfigurationRepository& configuration) = 0;
    };
}
