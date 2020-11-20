#pragma once

#include <vector>
#include <string>
#include <cstring>

#include "ProcessCallback.h"

#include "ParticleCommon.h"
#include "ParticleNode.h"
#include "ParticleOperation.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::vector;
    using std::string;
    using std::memset;
    using std::memcpy;

    using ::embeddedpenguins::modelengine::threads::ProcessCallback;

    class ParticleSupport
    {
        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        vector<unsigned long long int> initializedCells_ {};

    public:
        ParticleSupport() :
            width_(100),
            height_(100)
        {

        }

        ParticleSupport(unsigned long width, unsigned long height) :
            width_(width),
            height_(height)
        {

        }

        ParticleSupport(const ParticleSupport& other) :
            width_(other.width_),
            height_(other.height_)
        {

        }

        void MakeStopSignal(unsigned long long int index)
        {
        }

        void InitializeCell(vector<ParticleNode>& model, const string& name, unsigned long int row, unsigned long int column, int horizontalVector, int verticalVector, int mass, int speed, ParticleType type)
        {
            auto index = row * width_ + column;
            auto& particleNode = model[index];

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

        void SignalInitialCells(vector<ParticleNode>& model, ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            for (auto index : initializedCells_)
            {
                auto& particleNode = model[index];
                string particleName(particleNode.Name);
                callback(ParticleOperation(index, particleName));
            }
        }
    };
}
