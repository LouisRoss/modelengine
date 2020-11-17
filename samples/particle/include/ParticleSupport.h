#pragma once

#include <vector>

#include "ProcessCallback.h"

#include "ParticleCommon.h"
#include "ParticleNode.h"
#include "ParticleOperation.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::vector;

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

        void InitializeCell(vector<ParticleNode>& model, unsigned long int row, unsigned long int column, int horizontalVector, int verticalVector, int mass, int speed)
        {
            auto index = row * width_ + column;
            auto& particleNode = model[index];

            particleNode.Occupied = true;
            particleNode.HorizontalVector = horizontalVector;
            particleNode.VerticalVector = verticalVector;
            particleNode.Mass = mass;
            particleNode.Speed = speed;

            initializedCells_.push_back(index);
        }

        void SignalInitialCells(ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            for (auto index : initializedCells_)
            {
                callback(ParticleOperation(index));
            }
        }
    };
}
