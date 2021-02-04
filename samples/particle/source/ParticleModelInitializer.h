#pragma once

#include "ModelEngineCommon.h"
#include "sdk/ModelInitializer.h"

#include "ParticleOperation.h"
#include "ParticleSupport.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using embeddedpenguins::modelengine::ConfigurationUtilities;
    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ParticleModelInitializer : public ModelInitializer<ParticleOperation, ParticleSupport, ParticleRecord>
    {
    public:
        ParticleModelInitializer(ConfigurationUtilities& configuration, ParticleSupport helper) :
            ModelInitializer<ParticleOperation, ParticleSupport, ParticleRecord>(configuration, helper)
        {
        }

        virtual void Initialize() override
        {
            helper_.InitializeModel();
        }
    };
}
