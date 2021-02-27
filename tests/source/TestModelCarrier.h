#pragma once

#include <vector>

#include "TestNode.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using std::vector;

    struct TestModelCarrier
    {
        vector<TestNode>& Model;
        unsigned long int ModelSize() { return Model.size(); }
        bool Valid { true };
    };
}
