#include "LifeSupport.h"
#include "ModelLifeInitializer.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::vector;

    using embeddedpenguins::modelengine::sdk::IModelInitializer;
    
    ModelLifeInitializer::ModelLifeInitializer(LifeModelCarrier carrier, json& configuration) :
        ModelInitializer(configuration, LifeSupport(carrier, configuration))
    {
    }

    void ModelLifeInitializer::Initialize()
    {
        helper_.InitializeModel();

        initializedCells_.clear();

        auto centerCell = (helper_.Width() * helper_.Height() / 2) + (helper_.Width() / 2);

        helper_.MakeGlider(centerCell, initializedCells_);
        helper_.MakeStopSignal(centerCell - (helper_.Width() * 10), initializedCells_);
        helper_.MakeStopSignal(centerCell - (helper_.Width() * 5) - 15, initializedCells_);
        helper_.InitializeCells(initializedCells_);
    }

    void ModelLifeInitializer::InjectSignal(ProcessCallback<LifeOperation, LifeRecord>& callback)
    {
        helper_.SignalInitialCells(initializedCells_, callback);
    }

    // the class factories

    extern "C" IModelInitializer<LifeOperation, LifeRecord>* create(LifeModelCarrier carrier, json& configuration) {
        return new ModelLifeInitializer(carrier, configuration);
    }

    extern "C" void destroy(IModelInitializer<LifeOperation, LifeRecord>* p) {
        delete p;
    }
}
