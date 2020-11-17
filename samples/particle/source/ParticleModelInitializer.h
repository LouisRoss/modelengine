#pragma once

#include <vector>
#include <chrono>

#include "nlohmann/json.hpp"

#include "ModelEngine.h"
#include "sdk/ModelInitializer.h"

#include "ParticleNode.h"
#include "ParticleOperation.h"
#include "ParticleImplementation.h"
#include "ParticleRecord.h"
#include "ParticleSupport.h"

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

    class ParticleModelInitializer : public ModelInitializer<ParticleNode, ParticleOperation, ParticleRecord>
    {
        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };
        ParticleSupport support_ { };

    public:
        ParticleModelInitializer(vector<ParticleNode>& model, json& configuration);
        virtual void Initialize() override;
        virtual void InjectSignal(ProcessCallback<ParticleOperation, ParticleRecord>& callback) override;
    };
}
