#pragma once

#include <string>
#include <sstream>

#include "ParticleCommon.h"
#include "ParticleNode.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::string;
    using std::ostringstream;

    enum class ParticleRecordType
    {
        Propagate,
        Land,
        Collide
    };

    struct ParticleRecordEvaluation
    {
        unsigned long long int ParticleIndex { };
    };

    struct ParticleRecord
    {
        ParticleRecordType Type { ParticleRecordType::Propagate };
        unsigned long long int ParticleIndex { };
        int HorizontalVector { };
        int VerticalVector { };
        int Mass { };
        int Speed { };
        bool Occupied { false };

        ParticleRecord(ParticleRecordType type, unsigned long long int particleIndex, const ParticleNode& particleNode) :
            Type(type),
            ParticleIndex(particleIndex),
            HorizontalVector(particleNode.HorizontalVector),
            VerticalVector(particleNode.VerticalVector),
            Mass(particleNode.Mass),
            Speed(particleNode.Speed),
            Occupied(particleNode.Occupied)
        {
        }

        static const string Header()
        {
            ostringstream header;
            header << "Particle-Event-Type,Particle-Index,Particle-Occupied,horizontal-vector,vertical-vector,mass,speed";
            return header.str();
        }

        const string Format()
        {
            ostringstream row;
            row << (int)Type << "," << ParticleIndex << "," << Occupied << ",";
            switch (Type)
            {
                case ParticleRecordType::Propagate:
                case ParticleRecordType::Land:
                    row << HorizontalVector << ',' << VerticalVector << ',' << Mass << ',' << Speed;
                    break;

                default:
                    row << "N/A,N/A,N/A,N/A";
                    break;
            }

            return row.str();
        }
    };
}
