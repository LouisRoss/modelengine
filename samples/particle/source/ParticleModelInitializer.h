#pragma once

#include <vector>
#include <chrono>

#include "nlohmann/json.hpp"

#include "ModelEngine.h"
#include "sdk/ModelInitializer.h"

#include "ParticleNode.h"
#include "ParticleOperation.h"
#include "ParticleSupport.h"
#include "ParticleImplementation.h"
#include "ParticleRecord.h"
#include "ParticleModelCarrier.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::vector;
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;
    using std::chrono::hours;
    using std::chrono::duration_cast;

    using nlohmann::json;

    using embeddedpenguins::modelengine::ModelEngine;
    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ParticleModelInitializer : public ModelInitializer<ParticleOperation, ParticleSupport, ParticleRecord>
    {
    public:
        ParticleModelInitializer(ParticleModelCarrier carrier, json& configuration);
        virtual void Initialize() override;
        virtual void InjectSignal(ProcessCallback<ParticleOperation, ParticleRecord>& callback) override;
    };
}
