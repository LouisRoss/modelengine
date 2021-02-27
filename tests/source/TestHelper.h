#pragma once

#include <iostream>
#include <vector>

#include "nlohmann/json.hpp"

#include "ConfigurationRepository.h"
#include "ProcessCallback.h"

#include "TestModelCarrier.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using std::cout;
    using std::vector;

    using nlohmann::json;

    using ::embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;

    class TestHelper
    {
        TestModelCarrier& modelCarrier_;
        const ConfigurationRepository& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };

    public:
        TestModelCarrier& Model() { return modelCarrier_; }
        const ConfigurationRepository& Configuration() const { return configuration_; }

    public:
        TestHelper(TestModelCarrier& modelCarrier, const ConfigurationRepository& configuration) :
            modelCarrier_(modelCarrier),
            configuration_(configuration)
        {
            const json& modelJson = configuration_.Configuration()["Model"];
            if (!modelJson.is_null() && modelJson.contains("Dimensions"))
            {
                auto dimensionElement = modelJson["Dimensions"];
                if (dimensionElement.is_array())
                {
                    auto dimensionArray = dimensionElement.get<vector<int>>();
                    width_ = dimensionArray[0];
                    height_ = dimensionArray[1];
                }
            }
        }

        const unsigned long int Width() const { return width_; }
        const unsigned long int Height() const { return height_; }

        bool AllocateModel(unsigned long int modelSize = 0)
        {
            if (!CreateModel(modelSize))
            {
                cout << "Unable to create model of size " << modelSize << "\n";
                return false;
            }

            return true;
        }

        void InitializeModel()
        {
            LoadOptionalDimensions();
        }

    private:
        //
        // Allocate memory for the model.
        // NOTE: Only to be called from the main process, not a load library.
        //
        bool CreateModel(unsigned long int modelSize)
        {
            auto size = modelSize;
            if (size == 0)
            {
                const json& modelJson = configuration_.Configuration()["Model"];
                if (!modelJson.is_null() && modelJson.contains("ModelSize"))
                {
                    const json& modelSizeJson = modelJson["ModelSize"];
                    if (modelSizeJson.is_number_unsigned())
                        size = modelSizeJson.get<unsigned int>();
                }
            }

            if (size == 0)
            {
                cout << "No model size configured or supplied, initializer cannot create model\n";
                modelCarrier_.Valid = false;
                return false;
            }

            modelCarrier_.Model.resize(size);

            return true;
        }

        void LoadOptionalDimensions()
        {
            // Override the dimension defaults if configured.
            const json& configuration = configuration_.Configuration();
            auto& modelSection = configuration["Model"];
            if (!modelSection.is_null() && modelSection.contains("Dimensions"))
            {
                auto dimensionElement = modelSection["Dimensions"];
                if (dimensionElement.is_array())
                {
                    auto dimensionArray = dimensionElement.get<std::vector<int>>();
                    width_ = dimensionArray[0];
                    height_ = dimensionArray[1];
                }
            }

            maxIndex_ = width_ * height_;
        }
    };
}
