#pragma once

#include <vector>
#include <chrono>

#include "nlohmann/json.hpp"

#include "ModelEngine.h"
#include "sdk/ModelInitializer.h"

#include "LifeNode.h"
#include "LifeOperation.h"
#include "LifeImplementation.h"
#include "LifeRecord.h"

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

    class ModelLifeInitializer : public ModelInitializer<LifeNode, LifeOperation, LifeImplementation, LifeRecord>
    {
        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };

    public:
        ModelLifeInitializer(vector<LifeNode>& model, json& configuration);
        virtual void Initialize() override;
        virtual void InjectSignal(ProcessCallback<LifeOperation, LifeRecord>& callback) override;
    };
}
