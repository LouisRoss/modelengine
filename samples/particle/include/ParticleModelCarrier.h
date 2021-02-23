#pragma once

#include <vector>

#include "ParticleNode.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::vector;

    struct ParticleModelCarrier
    {
        vector<ParticleNode>& Model;
        unsigned long int ModelSize() { return Model.size(); }
        bool Valid { true };
    };
}
