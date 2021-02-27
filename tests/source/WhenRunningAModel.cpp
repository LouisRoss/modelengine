#include <memory>
#include <chrono>

#include "nlohmann/json.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestConfigurationRepository.h"
#include "ModelEngine.h"
#include "TestNode.h"
#include "TestOperation.h"
#include "TestImplementation.h"
#include "TestIdleImplementation.h"
#include "TestModelCarrier.h"
#include "TestHelper.h"

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

  using nlohmann::json;

  //using ::embeddedpenguins::core::neuron::model::ConfigurationRepository;
  using ::embeddedpenguins::modelengine::ModelEngine;

  constexpr unsigned long long int modelSize_ = 5'000;
  string TestConfigurationWhenRunningAModel = "\
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

  string TestControlWhenRunningAModel = "\
{\
\"Configuration\": \"life1/life.json\",\
\"Monitor\": \"life1/center.json\"\
}";


  class WhenRunningAModel : public ::testing::Test
  {
  protected:
    //unique_ptr<vector<TestNode>> model_ = make_unique<vector<TestNode>>(modelSize_);
    TestConfigurationRepository configuration_;
    vector<TestNode> model_;
    TestModelCarrier carrier_ { .Model = model_ };
    unique_ptr<ModelEngine<TestOperation, TestImplementation, TestHelper, TestRecord>> modelEngine_ { };
    nanoseconds duration_ { std::chrono::nanoseconds::min() };
    TestHelper helper_;

    vector<TestNode>& GetModel() { return model_; }
    ModelEngine<TestOperation, TestImplementation, TestHelper, TestRecord>& GetModelEngine() { return *modelEngine_; }
    void SetConfiguredModelTicks(unsigned int periodInMicroseconds) { configuration_.Configuration()["Model"]["ModelTicks"] = periodInMicroseconds; }

    WhenRunningAModel() :
      configuration_(TestConfigurationWhenRunningAModel, TestControlWhenRunningAModel),
      helper_(carrier_, configuration_)
    {
      model_.resize(modelSize_);
    }

    ~WhenRunningAModel() override { }
    void SetUp() override { }
    void TearDown() override { }

    void SetModelEngine(unsigned long long int size)
    {
      model_.resize(size);
      modelEngine_.reset();
      modelEngine_ = make_unique<ModelEngine<TestOperation, TestImplementation, TestHelper, TestRecord>>(helper_, configuration_);
      duration_ = nanoseconds::min();
    }

    void SetModelEngine(unique_ptr<ModelEngine<TestOperation, TestImplementation, TestHelper, TestRecord>>& modelEngine)
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
    ModelEngine<TestOperation, TestIdleImplementation, TestHelper, TestRecord> modelEngine(helper_, configuration_);

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
    SetConfiguredModelTicks(1000);
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    EXPECT_NEAR(modelEngine_->GetIterations(), 50, 1);
  }

  TEST_F(WhenRunningAModel, ModelEngineReturnsCorrectWorkItems)
  {
    // Arrange
    SetConfiguredModelTicks(1000);
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
    SetConfiguredModelTicks(1000);
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    EXPECT_NEAR(std::chrono::duration_cast<milliseconds>(duration_).count(), 50, 2);
  }

  TEST_F(WhenRunningAModel, ModelEngineInitializeWasCalled)
  {
    // Arrange
    SetConfiguredModelTicks(1000);
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    EXPECT_PRED1([this](auto v) { return (v >= 1 && v <= modelEngine_->GetWorkerCount()); }, GetModel()[1].Data);
  }

  TEST_F(WhenRunningAModel, ModelEngineWorkersDoCorrectWork)
  {
    // Arrange
    SetConfiguredModelTicks(1000);
    SetModelEngine(5'000);

    // Act
    RunModelEngine(50);

    // Assert
    auto workerCount = modelEngine_->GetWorkerCount();
    auto iterations = modelEngine_->GetIterations();

    // The first iteration is the initialization.
    // The second iteration does no recorded work, but signals one work item for each Id.
    // The third iteration does one work item per thread, and signals 7 work items per Id.
    // The subsequent iterations - 3 iterations do 7 work items per thread and signal 7 work items per Id.
    // A work item adds its id to the index in the model for each work item.
    // So, the expected number in each index is Id * (workerCount * (iterations - 3) + 1)
    auto baseIncrement = workerCount * (iterations - 3) + 1;
    auto index = 0;
    auto span = carrier_.ModelSize() / workerCount;

    for (auto id = 1; id < workerCount; id++)
    {
      EXPECT_EQ(carrier_.Model[index].Data, baseIncrement * id);
      index += span;
    }
    EXPECT_EQ(carrier_.Model[carrier_.ModelSize() - 1].Data, baseIncrement * workerCount);
  }
}
