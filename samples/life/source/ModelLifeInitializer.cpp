#include "ConfigurationRepository.h"

#include "ModelLifeInitializer.h"

#include "LifeModelCarrier.h"
#include "LifeOperation.h"
#include "LifeRecord.h"
#include "LifeSupport.h"

namespace embeddedpenguins::life::infrastructure
{
    using embeddedpenguins::core::neuron::model::ConfigurationRepository;

    using embeddedpenguins::core::neuron::model::IModelInitializer;
    
    // the class factories

    extern "C" IModelInitializer<LifeSupport>* create(LifeSupport& helper) {
        return new ModelLifeInitializer(helper);
    }

    extern "C" void destroy(IModelInitializer<LifeSupport>* p) {
        delete p;
    }
}
