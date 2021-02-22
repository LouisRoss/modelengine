#pragma once

#include "ConfigurationRepository.h"
#include "sdk/ModelInitializer.h"

#include "ParticleOperation.h"
#include "ParticleSupport.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ParticleModelInitializer : public ModelInitializer<ParticleSupport>
    {
    public:
        ParticleModelInitializer(ConfigurationRepository& configuration, ParticleSupport helper) :
            ModelInitializer<ParticleSupport>(configuration, helper)
        {
        }

        virtual void Initialize() override
        {
            helper_.InitializeModel();
        }
    };
}
