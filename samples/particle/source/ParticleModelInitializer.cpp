#include <string>
#include <ostream>

#include "ParticleSupport.h"
#include "ParticleModelInitializer.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::string;
    using std::ostringstream;

    using embeddedpenguins::modelengine::sdk::IModelInitializer;
    
    ParticleModelInitializer::ParticleModelInitializer(ParticleModelCarrier carrier, json& configuration) :
        ModelInitializer(configuration, ParticleSupport(carrier, configuration))
    {
    }

    void ParticleModelInitializer::Initialize()
    {
        helper_.InitializeModel();

        auto centerCell = (helper_.Width() * helper_.Height() / 2) + (helper_.Width() / 2);

        int verticalVector = -3;
        int horizontalVector = -3;
        int mass = 5;
        int speed = 0;
        ParticleType type = ParticleType::Neutron;

        for (auto row = 0; row < helper_.Height(); row += 8)
        {
            for (auto column = 0; column < helper_.Width(); column += 8)
            {
                ostringstream nameStream;
                nameStream << "P(" << std::setw(3) << std::setfill('0') << row << ',' << std::setw(3) << std::setfill('0') << column << ')';
                helper_.InitializeCell(nameStream.str(), row, column, verticalVector, horizontalVector, mass, speed, type);

                verticalVector++;
                if (verticalVector > 3)
                {
                    verticalVector = -3;
                    horizontalVector++;
                    if (horizontalVector > 3) horizontalVector = -3;
                }
                if (verticalVector == 0 && horizontalVector == 0) verticalVector++;
                speed++;
                if (speed > 9) speed = 0;

                auto nextType = (int)type + 1;
                if (nextType > (int)ParticleType::Photon) nextType = 0;
                type = (ParticleType)nextType;
            }
        }
    }

    void ParticleModelInitializer::InjectSignal(ProcessCallback<ParticleOperation, ParticleRecord>& callback)
    {
        helper_.SignalInitialCells(callback);
    }

    // the class factories

    extern "C" IModelInitializer<ParticleOperation, ParticleRecord>* create(ParticleModelCarrier carrier, json& configuration) {
        return new ParticleModelInitializer(carrier, configuration);
    }

    extern "C" void destroy(IModelInitializer<ParticleOperation, ParticleRecord>* p) {
        delete p;
    }
}
