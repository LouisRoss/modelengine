#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "ConfigurationRepository.h"
#include "ProcessCallback.h"

#include "TestModelCarrier.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using std::cout;
    using std::istringstream;
    using std::string;
    using std::vector;

    using nlohmann::json;

    using ::embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;

    class TestConfigurationRepository : public ConfigurationRepository
    {
    public:
        json& Configuration() { return configuration_; }

        TestConfigurationRepository() = default;

        TestConfigurationRepository(const string& configurationContent)
        {
            configuration_ = json::parse(configurationContent);
            //cout << configuration_.dump(4) << "\n";
        }

        TestConfigurationRepository(const string& configurationContent, const string& controlContent)
        {
            configuration_ = json::parse(configurationContent);
            control_ = json::parse(controlContent);
        }
    };
}
