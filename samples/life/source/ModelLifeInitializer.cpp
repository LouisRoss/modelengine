#include "ModelLifeInitializer.h"

namespace embeddedpenguins::life::infrastructure
{
    using embeddedpenguins::modelengine::sdk::IModelInitializer;
    
    ModelLifeInitializer::ModelLifeInitializer(vector<LifeNode>& model, json& configuration) :
        ModelInitializer(model, configuration)
    {
    }

    void ModelLifeInitializer::Initialize()
    {
        cout << "Enter Initializer::Initialize()\n";

        auto dimensionElement = configuration_["Model"]["Dimensions"];
        if (dimensionElement.is_array())
        {
            auto dimensionArray = dimensionElement.get<vector<int>>();
            width_ = dimensionArray[0];
            height_ = dimensionArray[1];
        }

        auto modelSize = width_ * height_;
        cout << "Using width = " << width_ << ", height = " << height_ << ", modelsize = " << modelSize << "\n";
        model_.resize(modelSize);

        auto centerCell = (width_ * height_ / 2) + (width_ / 2);
        model_[centerCell - 1].Alive = true;
        model_[centerCell].Alive = true;
        model_[centerCell + 1].Alive = true;
        //model_[centerCell + width_ + 1].Alive = true;
        //model_[centerCell + (width_ * 2)].Alive = true;
    }

    void ModelLifeInitializer::InjectSignal(ProcessCallback<LifeOperation, LifeRecord>& callback)
    {
        auto centerCell = (width_ * height_ / 2) + (width_ / 2);

        callback(LifeOperation(centerCell - 1, Operation::Evaluate));
        callback(LifeOperation(centerCell, Operation::Evaluate));
        callback(LifeOperation(centerCell + 1, Operation::Evaluate));

        callback(LifeOperation(centerCell - width_, Operation::Evaluate));
        callback(LifeOperation(centerCell + width_, Operation::Evaluate));

        //callback(LifeOperation(centerCell + width_ + 1, Operation::Evaluate));
        //callback(LifeOperation(centerCell + (width_ * 2), Operation::Evaluate));
    }

    // the class factories

    extern "C" IModelInitializer<LifeNode, LifeOperation, LifeImplementation, LifeRecord>* create(vector<LifeNode>& model, json& configuration) {
        return new ModelLifeInitializer(model, configuration);
    }

    extern "C" void destroy(IModelInitializer<LifeNode, LifeOperation, LifeImplementation, LifeRecord>* p) {
        delete p;
    }
}
