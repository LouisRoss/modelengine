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

    extern "C" IModelInitializer<ParticleSupport>* create(ParticleModelCarrier& carrier, ConfigurationRepository& configuration) {
        return new ParticleModelInitializer(configuration, ParticleSupport(carrier, configuration));
    }

    extern "C" void destroy(IModelInitializer<ParticleSupport>* p) {
        delete p;
    }
}
