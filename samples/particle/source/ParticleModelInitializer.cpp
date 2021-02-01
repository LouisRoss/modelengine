#include "ParticleModelInitializer.h"

#include "ParticleModelCarrier.h"
#include "ParticleOperation.h"
#include "ParticleSupport.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using embeddedpenguins::modelengine::sdk::IModelInitializer;

    // the class factories

    extern "C" IModelInitializer<ParticleOperation, ParticleRecord>* create(ParticleModelCarrier& carrier, ConfigurationUtilities& configuration) {
        return new ParticleModelInitializer(configuration, ParticleSupport(carrier, configuration));
    }

    extern "C" void destroy(IModelInitializer<ParticleOperation, ParticleRecord>* p) {
        delete p;
    }
}
