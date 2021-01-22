#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "nlohmann/json.hpp"

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

    using ::embeddedpenguins::modelengine::threads::ProcessCallback;

    class ParticleSupport
    {
        ParticleModelCarrier modelCarrier_;
        json& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        vector<unsigned long long int> initializedCells_ {};

    public:
        const unsigned long int Width() const { return width_; }
        const unsigned long int Height() const { return height_; }

    public:
        ParticleSupport(ParticleModelCarrier modelCarrier, json& configuration) :
            modelCarrier_(modelCarrier),
            configuration_(configuration)
        {

        }

        void InitializeModel()
        {
            auto dimensionElement = configuration_["Model"]["Dimensions"];
            if (dimensionElement.is_array())
            {
                auto dimensionArray = dimensionElement.get<vector<int>>();
                width_ = dimensionArray[0];
                height_ = dimensionArray[1];
            }

            auto modelSize = width_ * height_;
            cout << "Using width = " << width_ << ", height = " << height_ << ", modelsize = " << modelSize << "\n";
            modelCarrier_.Model.resize(modelSize);
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
    };
}
