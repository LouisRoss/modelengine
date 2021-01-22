#pragma once

#include <vector>

#include "LifeNode.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::vector;

    struct LifeModelCarrier
    {
        vector<LifeNode>& Model;
        unsigned long int ModelSize() { return Model.size(); }
    };
}
