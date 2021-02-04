#pragma once

#include <vector>
#include <chrono>

#include "ModelEngineCommon.h"
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

    using embeddedpenguins::modelengine::ConfigurationUtilities;
    using embeddedpenguins::modelengine::ModelEngine;
    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ModelLifeInitializer : public ModelInitializer<LifeOperation, LifeSupport, LifeRecord>
    {
    public:
        ModelLifeInitializer(ConfigurationUtilities& configuration, LifeSupport helper) :
            ModelInitializer<LifeOperation, LifeSupport, LifeRecord>(configuration, helper)
        {

        }

        virtual void Initialize() override
        {
            helper_.InitializeModel();
        }
    };
}
