#include <memory>
#include <chrono>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ModelEngine.h"
#include "TestNode.h"
#include "TestOperation.h"
#include "TestImplementation.h"
#include "TestIdleImplementation.h"

namespace test::embeddedpenguins::modelengine::infrastructure
{
  using std::string;
  using std::vector;
  using std::unique_ptr;
  using std::make_unique;
  using std::this_thread::sleep_for;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;
  using std::chrono::microseconds;
  using std::chrono::nanoseconds;

  using ::embeddedpenguins::modelengine::ModelEngine;

  class WhenRunningAModel : public ::testing::Test
  {
  protected:
    unsigned long long int modelSize_ = 5'000;
    unique_ptr<vector<TestNode>> model_ = make_unique<vector<TestNode>>(modelSize_);
    unique_ptr<ModelEngine<TestNode, TestOperation, TestImplementation, TestRecord>> modelEngine_ { };
    nanoseconds duration_ { std::chrono::nanoseconds::min() };

    vector<TestNode>& GetModel() { return *model_; }
    ModelEngine<TestNode, TestOperation, TestImplementation, TestRecord>& GetModelEngine() { return *modelEngine_; }

    WhenRunningAModel()
    {
    }

    ~WhenRunningAModel() override { }
    void SetUp() override { }
    void TearDown() override { }

    void SetModelEngine(unsigned long long int size, int workerCount = 0)
    {
      model_->resize(size);
      modelEngine_.reset();
      modelEngine_ = make_unique<ModelEngine<TestNode, TestOperation, TestImplementation, TestRecord>>(*model_, microseconds(1'000), workerCount);
      duration_ = nanoseconds::min();
    }

    void SetModelEngine(unique_ptr<ModelEngine<TestNode, TestOperation, TestImplementation, TestRecord>>& modelEngine, int workerCount = 0)
    {
      modelEngine_.reset();
      modelEngine_ = std::move(modelEngine);
      duration_ = nanoseconds::min();
    }

    void RunModelEngine(int runTime)
    {
      if (!modelEngine_) FAIL() << "RunModelEngine called without initializing by calling SetModelEngine";

      modelEngine_->Run();
      if (runTime > 0) sleep_for(milliseconds(runTime));
      modelEngine_->WaitForQuit();

      duration_ = modelEngine_->GetDuration();
    }

    void RunStopModelEngine()
    {
      if (!modelEngine_) FAIL() << "RunStopModelEngine called without initializing by calling SetModelEngine";

      modelEngine_->Run();
      modelEngine_->WaitForQuit();
      duration_ = modelEngine_->GetDuration();
    }
  };

  TEST_F(WhenRunningAModel, ModelEngineHasCorrectDefaultWorkerThreadCount)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunStopModelEngine();

    // Assert
    auto coreCount = std::thread::hardware_concurrency();
    EXPECT_EQ(modelEngine_->GetWorkerCount(), coreCount - 1);
  }

  TEST_F(WhenRunningAModel, ModelEngineHasCorrectWorkerThreadCount)
  {
    // Arrange
    SetModelEngine(5'000, 2);

    // Act
    RunStopModelEngine();

    // Assert
    EXPECT_EQ(modelEngine_->GetWorkerCount(), 2);
  }

  TEST_F(WhenRunningAModel, ModelEngineStoppedReturnsZeroIterations)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunStopModelEngine();

    // Assert
    EXPECT_EQ(modelEngine_->GetIterations(), 0);
  }

  TEST_F(WhenRunningAModel, ModelEngineStoppedReturnsZeroSetOfWorkItems)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunStopModelEngine();

    // Assert
    auto iterations = modelEngine_->GetIterations();
    EXPECT_EQ(modelEngine_->GetTotalWork(), iterations*49);
  }

  TEST_F(WhenRunningAModel, ModelEngineRunsWithIdleCycles)
  {
    // Arrange
    ModelEngine<TestNode, TestOperation, TestIdleImplementation, TestRecord> modelEngine(*model_, microseconds(1'000));

    // Act
    modelEngine.Run();
    sleep_for(milliseconds(50));
    modelEngine.WaitForQuit();

    // Assert
    EXPECT_EQ(modelEngine.GetTotalWork(), 1);
  }

  TEST_F(WhenRunningAModel, ModelEngineReturnsCorrectIterations)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    EXPECT_NEAR(modelEngine_->GetIterations(), 50, 1);
  }

  TEST_F(WhenRunningAModel, ModelEngineReturnsCorrectWorkItems)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    auto iterations = modelEngine_->GetIterations();
    EXPECT_EQ(modelEngine_->GetTotalWork(), (iterations - 2) * 49 + 1 + 7);
  }

  TEST_F(WhenRunningAModel, ModelEngineTakesCorrectDuration)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    EXPECT_NEAR(std::chrono::duration_cast<milliseconds>(duration_).count(), 50, 2);
  }

  TEST_F(WhenRunningAModel, ModelEngineInitializeWasCalled)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    EXPECT_PRED1([this](auto v) { return (v >= 1 && v <= modelEngine_->GetWorkerCount()); }, GetModel()[1].Data);
  }

  TEST_F(WhenRunningAModel, ModelEngineWorkersDoCorrectWork)
  {
    // Arrange
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    auto workerCount = modelEngine_->GetWorkerCount();
    auto iterations = modelEngine_->GetIterations();
    auto baseIncrement = workerCount * (iterations - 1) - 6;
    auto index = 0;
    auto span = model_->size() / workerCount;

    for (auto id = 1; id < workerCount; id++)
    {
      EXPECT_EQ(GetModel()[index].Data, baseIncrement * id);
      index += span;
    }
    EXPECT_EQ(GetModel()[model_->size() - 1].Data, baseIncrement * workerCount);
  }
}
