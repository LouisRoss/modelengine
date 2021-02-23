#include "ConfigurationRepository.h"

#include "ParticleModelInitializer.h"

#include "ParticleModelCarrier.h"
#include "ParticleOperation.h"
#include "ParticleSupport.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using embeddedpenguins::core::neuron::model::IModelInitializer;

    // the class factories

    extern "C" IModelInitializer<ParticleSupport>* create(ParticleSupport& helper) {
        return new ParticleModelInitializer<ParticleSupport>(helper);
    }

    extern "C" void destroy(IModelInitializer<ParticleSupport>* p) {
        delete p;
    }
}
