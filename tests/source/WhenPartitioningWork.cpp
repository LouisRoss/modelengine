#include <vector>
#include <chrono>
#include <limits>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestNode.h"
#include "TestOperation.h"
#include "TestImplementation.h"
#include "TestRecord.h"

#include "ModelEngineContextOp.h"
#include "AdaptiveWidthPartitioner.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using time_point = std::chrono::high_resolution_clock::time_point;
    using std::chrono::milliseconds;
    using Clock = std::chrono::high_resolution_clock;
    using std::vector;
    using std::pair;
    using namespace ::embeddedpenguins::modelengine;

    class WhenPartitioningWork : public ::testing::Test
    {
    protected:
        vector<TestNode> model_ { };
        ModelEngineContext<TestNode, TestOperation, TestImplementation, TestRecord> context_ { };
        unsigned long long int now_{};

        WhenPartitioningWork()
        {
            context_.WorkerCount = std::thread::hardware_concurrency() - 1;
        }

        ~WhenPartitioningWork() override { }
        void SetUp() override { }
        void TearDown() override
        {
            for (auto& worker : context_.Workers)
                worker->Join();
        }
    };

    // Protected internal state is exposed to this derived class.
    class TestAdaptiveWidthPartitioner : public AdaptiveWidthPartitioner<TestNode, TestOperation, TestImplementation, TestRecord>
    {
    public:
        TestAdaptiveWidthPartitioner(ModelEngineContext<TestNode, TestOperation, TestImplementation, TestRecord>& context) :
            AdaptiveWidthPartitioner(context)
        {
            
        }

        vector<WorkItem<TestOperation>>& GetTotalSourceWork() { return totalSourceWork_; }

        typename vector<WorkItem<TestOperation>>::iterator FindCutoffPoint(unsigned long long int workCutoffTime)
        {
            return AdaptiveWidthPartitioner::FindCutoffPoint(workCutoffTime);
        }

        void AccumulateWorkFromAllWorkers() { AdaptiveWidthPartitioner::AccumulateWorkFromAllWorkers(); }

        void LoadWorkWithConsecutiveIndexes(int indexMax, unsigned long long int workTime)
        {
            LoadWorkWithSpacedIndexes(indexMax, workTime, 1);
        }

        void LoadWorkWithSpacedIndexes(int indexMax, unsigned long long int workTime, int span)
        {
            totalSourceWork_.clear();
            for (int index = 1; index <= indexMax; index++)
            {
                totalSourceWork_.push_back(WorkItem<TestOperation> { workTime, TestOperation(index * span) });
            }
        }

        void AddWorkWithSameIndex(int index, unsigned long long int workTime, int count)
        {
            for (int _ = count; _; _--)
            {
                totalSourceWork_.push_back(WorkItem<TestOperation> { workTime, TestOperation(index) });
            }
        }

        void LoadWorkWithSpacedIndexesAndTimes(int indexMax, int span, unsigned long long int workTime, unsigned long long int timeSpan)
        {
            totalSourceWork_.clear();
            auto execTime = workTime;
            for (int index = 1; index <= indexMax; index++, execTime += timeSpan)
            {
                totalSourceWork_.push_back(WorkItem<TestOperation> { execTime, TestOperation(index * span) });
            }
        }

    };

    TEST_F(WhenPartitioningWork, PastWorkIsDone)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);

        // Act
        partitioner.Partition(now_ + 20);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
    }

    TEST_F(WhenPartitioningWork, FutureWorkIsLeft)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);

        // Act
        partitioner.Partition(now_ + 5);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), std::thread::hardware_concurrency() - 1);
    }

    TEST_F(WhenPartitioningWork, PastWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);

        // Act
        partitioner.Partition(now_ + 20);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
        for (auto& worker : context_.Workers)
            EXPECT_EQ(worker->GetContext().WorkForThread.size(), 1);
    }

    TEST_F(WhenPartitioningWork, PastSparseWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithSpacedIndexes(std::thread::hardware_concurrency() - 1, now_ + 10, 1'000);

        // Act
        partitioner.Partition(now_ + 20);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
        for (auto& worker : context_.Workers)
            EXPECT_EQ(worker->GetContext().WorkForThread.size(), 1);
    }

    TEST_F(WhenPartitioningWork, PastSkewedWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);
        partitioner.AddWorkWithSameIndex(1, now_ + 10, 9);

        // Act
        partitioner.Partition(now_ + 20);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
        for (auto& worker : context_.Workers)
        {
            if (worker->GetContext().WorkerId == 1)
                EXPECT_EQ(worker->GetContext().WorkForThread.size(), 10);
            else
                EXPECT_LE(worker->GetContext().WorkForThread.size(), 2);
        }
    }

    TEST_F(WhenPartitioningWork, PastOddWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);
        partitioner.AddWorkWithSameIndex(std::thread::hardware_concurrency(), now_ + 10, 1);
        partitioner.AddWorkWithSameIndex(std::thread::hardware_concurrency() + 1, now_ + 10, 1);
        partitioner.AddWorkWithSameIndex(std::thread::hardware_concurrency() + 2, now_ + 10, 1);

        // Act
        partitioner.Partition(now_ + 20);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
        EXPECT_EQ(context_.Workers.size(), std::thread::hardware_concurrency() - 1);
        for (auto& worker : context_.Workers)
        {
            if (worker->GetContext().WorkerId == std::thread::hardware_concurrency() - 1)
                EXPECT_EQ(worker->GetContext().WorkForThread.size(), 4);
            else
                EXPECT_EQ(worker->GetContext().WorkForThread.size(), 1);
        }
    }

    TEST_F(WhenPartitioningWork, ShouldFindCorrectSplitTime)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithSpacedIndexesAndTimes(std::thread::hardware_concurrency() - 1, 10, now_, 10);

        // Act
        auto cutoff = partitioner.FindCutoffPoint(now_ + 35);

        // Assert
        EXPECT_EQ(cutoff - begin(partitioner.GetTotalSourceWork()), 4);
    }

    TEST_F(WhenPartitioningWork, ShouldFindSplitTimeForEmptyWork)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);

        // Act
        auto cutoff = partitioner.FindCutoffPoint(now_ + 1);

        // Assert
        EXPECT_EQ(cutoff, end(partitioner.GetTotalSourceWork()));
    }

    TEST_F(WhenPartitioningWork, ShouldFindSplitTimeForOutOfOrderWork)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.AddWorkWithSameIndex(1, now_ + 10, 1);
        partitioner.AddWorkWithSameIndex(1, now_ + 15, 1);
        partitioner.AddWorkWithSameIndex(1, now_ + 8, 1);
        partitioner.AddWorkWithSameIndex(1, now_ + 2, 1);
        partitioner.AddWorkWithSameIndex(1, now_ + 5, 1);

        // Act
        auto cutoff = partitioner.FindCutoffPoint(now_ + 6);

        // Assert
        EXPECT_EQ(cutoff - begin(partitioner.GetTotalSourceWork()), 2);
        for (auto work = begin(partitioner.GetTotalSourceWork()); work < cutoff; work++)
            EXPECT_LT(work->Tick, now_ + 6);
    }

    TEST_F(WhenPartitioningWork, ShouldNotIncludeCutoffTime)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithSpacedIndexesAndTimes(std::thread::hardware_concurrency() - 1, 10, now_, 10);

        // Act
        auto cutoff = partitioner.FindCutoffPoint(now_ + 40);

        // Assert
        EXPECT_EQ(cutoff - begin(partitioner.GetTotalSourceWork()), 4);
    }

    TEST_F(WhenPartitioningWork, WorkerWorkIsAccumulatedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        for (auto& worker : context_.Workers)
            for (int i = 0; i < context_.Workers.size(); i++)
                worker->GetContext().WorkForNextThread.push_back(WorkItem<TestOperation> { now_, TestOperation(i) });

        // Act
        partitioner.AccumulateWorkFromAllWorkers();

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), context_.Workers.size() * context_.Workers.size());
        for (auto& worker : context_.Workers)
            EXPECT_EQ(worker->GetContext().WorkForNextThread.size(), 0);
    }

    TEST_F(WhenPartitioningWork, WorkerWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        for (auto& worker : context_.Workers)
            for (int i = 0; i < context_.Workers.size(); i++)
                worker->GetContext().WorkForNextThread.push_back(WorkItem<TestOperation> { now_, TestOperation(i + 1) });

        // Act
        partitioner.Partition(now_ + 1);

        // Assert
        for (auto& worker : context_.Workers)
        {
            EXPECT_EQ(worker->GetContext().WorkForThread.size(), context_.Workers.size());
            for (auto& work : worker->GetContext().WorkForThread)
                EXPECT_EQ(work.Operator.Index, worker->GetContext().WorkerId);
        }
    }

    TEST_F(WhenPartitioningWork, DuplicateTimesAreAllowed)
    {
        // Arrange
        ModelEngineContextOp<TestNode, TestOperation, TestImplementation, TestRecord>(context_).CreateWorkers(model_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        auto expectedWorkItemCount {0};
        for (int i = 0; i < context_.Workers.size() * 100; i+=10)
        {
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 1), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 2), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 3), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 4), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 5), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 6), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 7), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 8), 1000 + i);
            ProcessCallback(context_.ExternalWorkSource)(TestOperation(i + 9), 1000 + i);
            expectedWorkItemCount += 10;
        }

        // Act
        partitioner.Partition(now_ + 2000);

        // Assert
        auto actualWorkItemCount {0};
        for (auto& worker : context_.Workers)
        {
            actualWorkItemCount += worker->GetContext().WorkForThread.size();
        }

        EXPECT_EQ(actualWorkItemCount, expectedWorkItemCount);
    }
}
