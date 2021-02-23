#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "nlohmann/json.hpp"

#include "ConfigurationRepository.h"
#include "ProcessCallback.h"

#include "ParticleCommon.h"
#include "ParticleNode.h"
#include "ParticleOperation.h"
#include "ParticleModelCarrier.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::cout;
    using std::vector;
    using std::string;
    using std::memset;
    using std::memcpy;

    using nlohmann::json;

    using embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using embeddedpenguins::modelengine::threads::ProcessCallback;

    class ParticleSupport
    {
        ParticleModelCarrier& modelCarrier_;
        const ConfigurationRepository& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };

        vector<unsigned long long int> initializedCells_ {};

    public:
        const unsigned long int Width() const { return width_; }
        const unsigned long int Height() const { return height_; }
        ParticleModelCarrier& Model() { return modelCarrier_; }
        const ConfigurationRepository& Configuration() const { return configuration_; }

    public:
        ParticleSupport(ParticleModelCarrier& modelCarrier, const ConfigurationRepository& configuration) :
            modelCarrier_(modelCarrier),
            configuration_(configuration)
        {
            auto dimensionElement = configuration_.Configuration()["Model"]["Dimensions"];
            if (dimensionElement.is_array())
            {
                auto dimensionArray = dimensionElement.get<vector<int>>();
                width_ = dimensionArray[0];
                height_ = dimensionArray[1];
            }
        }

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

        void InitializeCell(const string& name, unsigned long int row, unsigned long int column, int horizontalVector, int verticalVector, int mass, int speed, ParticleType type)
        {
            auto index = row * width_ + column;
            auto& particleNode = modelCarrier_.Model[index];

            memset(particleNode.Name, '\0', sizeof(particleNode.Name));
            memcpy(particleNode.Name, name.c_str(), (name.length() < sizeof(particleNode.Name) - 1 ? name.length() : sizeof(particleNode.Name) - 1));
            particleNode.Occupied = true;
            particleNode.HorizontalVector = horizontalVector;
            particleNode.VerticalVector = verticalVector;
            particleNode.Mass = mass;
            particleNode.Speed = speed;
            particleNode.Type = type;

            initializedCells_.push_back(index);
        }

        void SignalInitialCells(ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            for (auto index : initializedCells_)
            {
                auto& particleNode = modelCarrier_.Model[index];
                string particleName(particleNode.Name);
                callback(ParticleOperation(index, particleName));
            }
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
