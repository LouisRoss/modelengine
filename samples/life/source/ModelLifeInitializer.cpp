#include "ModelLifeInitializer.h"

#include "LifeModelCarrier.h"
#include "LifeOperation.h"
#include "LifeRecord.h"
#include "LifeSupport.h"

namespace embeddedpenguins::life::infrastructure
{
    using embeddedpenguins::modelengine::sdk::IModelInitializer;
    
    // the class factories

    extern "C" IModelInitializer<LifeOperation, LifeRecord>* create(LifeModelCarrier& carrier, ConfigurationUtilities& configuration) {
        return new ModelLifeInitializer(configuration, LifeSupport(carrier, configuration));
    }

    extern "C" void destroy(IModelInitializer<LifeOperation, LifeRecord>* p) {
        delete p;
    }
}
