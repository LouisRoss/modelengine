#pragma once

#include <vector>
#include <chrono>

#include "ConfigurationRepository.h"
#include "ModelEngine.h"
#include "sdk/ModelInitializer.h"

#include "LifeNode.h"
#include "LifeOperation.h"
#include "LifeSupport.h"
#include "LifeRecord.h"
#include "LifeModelCarrier.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::vector;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    using std::chrono::hours;
    using std::chrono::duration_cast;

    using embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using embeddedpenguins::modelengine::ModelEngine;
    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ModelLifeInitializer : public ModelInitializer<LifeSupport>
    {
    public:
        ModelLifeInitializer(LifeSupport helper) :
            ModelInitializer<LifeSupport>(helper)
        {

        }

        // Must be implemented, but not used unless implementation is a proxy.
        virtual void CreateProxy(LifeSupport helper) override
        {

        }

        virtual void Initialize() override
        {
            helper_.InitializeModel();
        }
    };
}
