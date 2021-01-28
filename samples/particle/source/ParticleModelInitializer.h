#pragma once

#include "nlohmann/json.hpp"

#include "sdk/ModelInitializer.h"

#include "ParticleOperation.h"
#include "ParticleSupport.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using nlohmann::json;

    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ParticleModelInitializer : public ModelInitializer<ParticleOperation, ParticleSupport, ParticleRecord>
    {
    public:
        ParticleModelInitializer(json& configuration, ParticleSupport helper) :
            ModelInitializer<ParticleOperation, ParticleSupport, ParticleRecord>(configuration, helper)
        {
        }

        virtual void Initialize() override
        {
            helper_.InitializeModel();
        }
    };
}
