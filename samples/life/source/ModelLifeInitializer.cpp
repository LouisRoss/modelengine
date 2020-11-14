#include "LifeSupport.h"
#include "ModelLifeInitializer.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::vector;

    using embeddedpenguins::modelengine::sdk::IModelInitializer;
    
    ModelLifeInitializer::ModelLifeInitializer(vector<LifeNode>& model, json& configuration) :
        ModelInitializer(model, configuration)
    {
    }

    void ModelLifeInitializer::Initialize()
    {
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

        initializedCells_.clear();

        auto centerCell = (width_ * height_ / 2) + (width_ / 2);

        LifeSupport lifeSupport(width_, height_);
        lifeSupport.MakeGlider(centerCell, initializedCells_);
        lifeSupport.MakeStopSignal(centerCell - (width_ * 10), initializedCells_);
        lifeSupport.MakeStopSignal(centerCell - (width_ * 5) - 15, initializedCells_);
        lifeSupport.InitializeCells(model_, initializedCells_);
    }

    void ModelLifeInitializer::InjectSignal(ProcessCallback<LifeOperation, LifeRecord>& callback)
    {
        LifeSupport(width_, height_).SignalInitialCells(initializedCells_, callback);
    }

    // the class factories

    extern "C" IModelInitializer<LifeOperation, LifeRecord>* create(vector<LifeNode>& model, json& configuration) {
        return new ModelLifeInitializer(model, configuration);
    }

    extern "C" void destroy(IModelInitializer<LifeOperation, LifeRecord>* p) {
        delete p;
    }
}
