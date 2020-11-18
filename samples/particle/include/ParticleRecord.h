#pragma once

#include <string>
#include <cstring>
#include <sstream>

#include "ParticleCommon.h"
#include "ParticleNode.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::string;
    using std::ostringstream;
    using std::memset;
    using std::memcpy;

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
        char Name[20] { };
        ParticleRecordType Type { ParticleRecordType::Propagate };
        unsigned long long int ParticleIndex { };
        int VerticalVector { };
        int HorizontalVector { };
        int Mass { };
        int Speed { };

        ParticleRecord(const string& name, ParticleRecordType type, unsigned long long int particleIndex, const ParticleNode& particleNode) :
            Type(type),
            ParticleIndex(particleIndex),
            VerticalVector(particleNode.VerticalVector),
            HorizontalVector(particleNode.HorizontalVector),
            Mass(particleNode.Mass),
            Speed(particleNode.Speed)
        {
            memset(Name, '\0', sizeof(Name));
            memcpy(Name, name.c_str(), (name.length() < 19 ? name.length() : 19));
        }

        ParticleRecord(const string& name, ParticleRecordType type, unsigned long long int particleIndex, int verticalVector, int horizontalVector, int mass, int speed) : 
            Type(type),
            ParticleIndex(particleIndex),
            VerticalVector(verticalVector),
            HorizontalVector(horizontalVector),
            Mass(mass),
            Speed(speed)
        {
            memset(Name, '\0', sizeof(Name));
            memcpy(Name, name.c_str(), (name.length() < 19 ? name.length() : 19));
        }

        static const string Header()
        {
            ostringstream header;
            header << "Particle-Name,Particle-Event-Type,Particle-Index,horizontal-vector,vertical-vector,mass,speed";
            return header.str();
        }

        const string Format()
        {
            ostringstream row;
            row << Name << "," << (int)Type << "," << ParticleIndex << ",";
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
