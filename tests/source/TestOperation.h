#pragma once

namespace test::embeddedpenguins::modelengine::infrastructure
{
    struct TestOperation
    {
        long long int Index {0};

        TestOperation() = default;
        TestOperation(long long int index) : Index(index) { }
    };
}
