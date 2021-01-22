#pragma once

#include <vector>
#include <chrono>

#include "nlohmann/json.hpp"

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

    using nlohmann::json;

    using embeddedpenguins::modelengine::ModelEngine;
    using embeddedpenguins::modelengine::sdk::ModelInitializer;

    class ModelLifeInitializer : public ModelInitializer<LifeNode, LifeOperation, LifeSupport, LifeRecord>
    {
        vector<unsigned long long int> initializedCells_ { };

    public:
        ModelLifeInitializer(LifeModelCarrier carrier, json& configuration);
        virtual void Initialize() override;
        virtual void InjectSignal(ProcessCallback<LifeOperation, LifeRecord>& callback) override;
    };
}
