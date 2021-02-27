#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Log.h"
#include "WorkerContext.h"
#include "ProcessCallback.h"
#include "TestOperation.h"
#include "TestRecord.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using std::chrono::microseconds;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::time_point_cast;

    using ::embeddedpenguins::modelengine::threads::WorkerContext;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;
    using ::embeddedpenguins::core::neuron::model::LogLevel;

    class WhenDoingSupportFunctions : public ::testing::Test
    {
    protected:
    };

    TEST_F(WhenDoingSupportFunctions, IntegerRolloverSubtractsCorrectly)
    {
        // arrange
        int64_t value1 = std::numeric_limits<unsigned int>::max() - 5;
        unsigned int smallValue1 = static_cast<unsigned int>(value1);

        // act
        int64_t value2 = value1 + 10;
        unsigned int smallValue2 = static_cast<unsigned int>(value2);
        
        // assert
        EXPECT_EQ(smallValue2, 4);
        EXPECT_EQ(value2 - value1, 10);
        EXPECT_EQ(smallValue2 - smallValue1, 10);
    }

    TEST_F(WhenDoingSupportFunctions, CallbackWorkHasCorrectTicks)
    {
        // arrange
        unsigned long long int iterations{};
        microseconds ticks{};
        LogLevel loggingLevel{};
        WorkerContext<TestOperation, TestRecord> context(iterations, ticks, loggingLevel);
        context.EnginePeriod = microseconds(1'000);
        ProcessCallback callback(context);

        // act
        callback.operator()(TestOperation(0), 25);

        ASSERT_EQ(context.WorkForFutureTicks1.size(), 1);

        auto expected = 24;
        auto actual = context.WorkForFutureTicks1[0].Tick;
        EXPECT_EQ(expected, actual);
    }
}
