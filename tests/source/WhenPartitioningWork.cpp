#include <vector>
#include <chrono>
#include <limits>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestNode.h"
#include "TestOperation.h"
#include "TestImplementation.h"
#include "TestModelCarrier.h"
#include "TestRecord.h"
#include "TestHelper.h"

#include "TestConfigurationRepository.h"
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

    string TestConfigurationPartitioningWork = "\
{\
\"Model\":\
{\
\"ModelTicks\": 50000,\
\"ModelSize\": 1000000,\
\"Dimensions\": [1000, 1000]\
},\
\"Execution\":\
{\
\"ModelExecutable\": \"./LifeModel\",\
\"ModelOptions\": \"\",\
\"InitializerLocation\": \"./ModelLifeInitializer.so\"\
},\
\"PostProcessing\":\
{\
\"RecordLocation\": \"../record/\",\
\"RecordFile\": \"ModelEngineRecord.csv\",\
\"CleanRecordFile\": \"CleanRecord.csv\",\
\"ImageFile\": \"spikes.png\"\
}\
}";

  string TestControlPartitioningWork = "\
{\
\"Configuration\": \"life1/life.json\",\
\"Monitor\": \"life1/center.json\"\
}";

    class WhenPartitioningWork : public ::testing::Test
    {
    protected:
        TestConfigurationRepository configuration_;
        vector<TestNode> model_ { };
        TestModelCarrier carrier_ { .Model = model_ };
        ModelEngineContext<TestOperation, TestImplementation, TestHelper, TestRecord> context_;
        TestHelper helper_;
        unsigned long long int now_{};

        WhenPartitioningWork() :
            configuration_(TestConfigurationPartitioningWork, TestControlPartitioningWork),
            helper_(carrier_, configuration_),
            context_(configuration_, helper_)
        {
            context_.WorkerCount = std::thread::hardware_concurrency() - 1;
        }

        ~WhenPartitioningWork() override { }
        void SetUp() override { }
        void TearDown() override
        {
            Log::Merge(context_.Logger);
            cout << "Writing log file to " << context_.LogFile << "... " << std::flush;
            Log::Print(context_.LogFile.c_str());
            cout << "Done\n";

            for (auto& worker : context_.Workers)
                worker->Join();
        }
    };

    // Protected internal state is exposed to this derived class.
    class TestAdaptiveWidthPartitioner : public AdaptiveWidthPartitioner<TestOperation, TestImplementation, TestHelper, TestRecord>
    {
        unsigned long int collectedWorkForWorkers_ {};

    public:
        TestAdaptiveWidthPartitioner(ModelEngineContext<TestOperation, TestImplementation, TestHelper, TestRecord>& context) :
            AdaptiveWidthPartitioner(context)
        {
            
        }

        vector<WorkItem<TestOperation>>& GetTotalSourceWork() { return totalSourceWork_; }
        vector<WorkItem<TestOperation>>& GetWorkForNextTick() { return workForNextTick_; };
        const unsigned long int CollectedWorkForWorkers() const { return collectedWorkForWorkers_; }
        unsigned long int& CollectedWorkForWorkers() { return collectedWorkForWorkers_; }

        typename vector<WorkItem<TestOperation>>::iterator FindCutoffPoint()
        {
            return AdaptiveWidthPartitioner::FindCutoffPoint();
        }

        void AccumulateWorkForNextTickFromAllWorkers() { AdaptiveWidthPartitioner::AccumulateWorkForNextTickFromAllWorkers(); }

        unsigned long int Partition(unsigned int count = 1) 
        {
            unsigned long int totalWork {};
            for (auto _ = count; _--;)
            {
                AdaptiveWidthPartitioner::ConcurrentPartitionStep();
                totalWork += AdaptiveWidthPartitioner::SingleThreadPartitionStep();
                ++context_.Iterations;

                // WorkForThread will be cleared on the next iteration, so accumulate here.
                for (auto& worker : context_.Workers)
                {
                    collectedWorkForWorkers_ += worker->GetContext().WorkForThread.size();
                }
            }

            return totalWork;
        }

        void LoadWorkWithConsecutiveIndexes(int indexMax, unsigned long long int workTick)
        {
            LoadWorkWithSpacedIndexes(indexMax, workTick, 1);
        }

        void LoadWorkWithSpacedIndexes(int indexMax, unsigned long long int workTick, int span)
        {
            totalSourceWork_.clear();
            for (int index = 1; index <= indexMax; index++)
            {
                totalSourceWork_.push_back(WorkItem<TestOperation> { workTick, TestOperation(index * span) });
            }
        }

        void AddWorkWithSameIndex(int index, unsigned long long int workTick, int count)
        {
            for (int _ = count; _; _--)
            {
                totalSourceWork_.push_back(WorkItem<TestOperation> { workTick, TestOperation(index) });
            }
        }

        void LoadWorkWithSpacedIndexesAndTimes(int indexMax, int span, unsigned long long int workTick, unsigned long long int tickSpan)
        {
            totalSourceWork_.clear();
            auto execTick = workTick;
            for (int index = 1; index <= indexMax; index++, execTick += tickSpan)
            {
                totalSourceWork_.push_back(WorkItem<TestOperation> { execTick, TestOperation(index * span) });
            }
        }

    };

    TEST_F(WhenPartitioningWork, PastWorkIsDone)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_);

        // Act
        partitioner.Partition();

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
    }

    TEST_F(WhenPartitioningWork, FutureWorkIsLeft)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);

        // Act
        partitioner.Partition();

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), std::thread::hardware_concurrency() - 1);
    }

    TEST_F(WhenPartitioningWork, PastWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);

        // Act
        partitioner.Partition(now_ + 11);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
        for (auto& worker : context_.Workers)
            EXPECT_EQ(worker->GetContext().WorkForThread.size(), 1);
    }

    TEST_F(WhenPartitioningWork, PastSparseWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithSpacedIndexes(std::thread::hardware_concurrency() - 1, now_ + 10, 1'000);

        // Act
        partitioner.Partition(now_ + 11);

        // Assert
        EXPECT_EQ(partitioner.GetTotalSourceWork().size(), 0);
        for (auto& worker : context_.Workers)
            EXPECT_EQ(worker->GetContext().WorkForThread.size(), 1);
    }

    TEST_F(WhenPartitioningWork, PastSkewedWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);
        partitioner.AddWorkWithSameIndex(1, now_ + 10, 9);

        // Act
        partitioner.Partition(now_ + 11);

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
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        partitioner.LoadWorkWithConsecutiveIndexes(std::thread::hardware_concurrency() - 1, now_ + 10);
        partitioner.AddWorkWithSameIndex(std::thread::hardware_concurrency(), now_ + 10, 1);
        partitioner.AddWorkWithSameIndex(std::thread::hardware_concurrency() + 1, now_ + 10, 1);
        partitioner.AddWorkWithSameIndex(std::thread::hardware_concurrency() + 2, now_ + 10, 1);

        // Act
        partitioner.Partition(now_ + 11);

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
        context_.Iterations += 31;
        auto cutoff = partitioner.FindCutoffPoint();

        // Assert
        EXPECT_EQ(cutoff - begin(partitioner.GetTotalSourceWork()), 4);
    }

    TEST_F(WhenPartitioningWork, ShouldFindSplitTimeForEmptyWork)
    {
        // Arrange
        TestAdaptiveWidthPartitioner partitioner(context_);

        // Act
        context_.Iterations += 1;
        auto cutoff = partitioner.FindCutoffPoint();

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
        context_.Iterations += 6;
        auto cutoff = partitioner.FindCutoffPoint();

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
        context_.Iterations += 39;
        auto cutoff = partitioner.FindCutoffPoint();

        // Assert
        EXPECT_EQ(cutoff - begin(partitioner.GetTotalSourceWork()), 4);
    }

    TEST_F(WhenPartitioningWork, WorkerWorkIsAccumulatedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        for (auto& worker : context_.Workers)
            for (int i = 0; i < context_.Workers.size(); i++)
                worker->GetContext().WorkForTick1.push_back(WorkItem<TestOperation> { now_, TestOperation(i) });

        // Act
        partitioner.AccumulateWorkForNextTickFromAllWorkers();

        // Assert
        EXPECT_EQ(partitioner.GetWorkForNextTick().size(), context_.Workers.size() * context_.Workers.size());
        for (auto& worker : context_.Workers)
            EXPECT_EQ(worker->GetContext().WorkForTick1.size(), 0);
    }

    TEST_F(WhenPartitioningWork, WorkerWorkIsPartitionedCorrectly)
    {
        // Arrange
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);
        for (auto& worker : context_.Workers)
            for (int i = 0; i < context_.Workers.size(); i++)
                worker->GetContext().WorkForTick1.push_back(WorkItem<TestOperation> { now_, TestOperation(i + 1) });

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
        ModelEngineContextOp<TestOperation, TestImplementation, TestHelper, TestRecord>(context_).CreateWorkers(helper_);
        TestAdaptiveWidthPartitioner partitioner(context_);

        // Acting as a worker thread, use buffer 1 to insert into.
        context_.ExternalWorkSource.CurrentBuffer = CurrentBufferType::Buffer1Current;
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
        partitioner.CollectedWorkForWorkers() = 0;

        // Acting as a model thread, use buffer 1 to extract from.
        context_.ExternalWorkSource.CurrentBuffer = CurrentBufferType::Buffer2Current;
        auto totalWork = partitioner.Partition(now_ + 2000);

        // Assert
        EXPECT_EQ(totalWork, expectedWorkItemCount);
        EXPECT_EQ(partitioner.CollectedWorkForWorkers(), expectedWorkItemCount);
    }
}
