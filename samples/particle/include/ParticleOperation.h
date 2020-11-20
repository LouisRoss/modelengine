#pragma once

#include <string>
#include <cstring>

#include "ParticleCommon.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::string;
    using std::memset;
    using std::memcpy;

    struct ParticleOperation
    {
        char Name[20] { };
        unsigned long long int Index { 0 };
        Operation Op;
        int VerticalVector {};
        int HorizontalVector {};
        int Gradient {};
        int Mass {};
        int Speed {};
        ParticleType Type { ParticleType::Photon };

        ParticleOperation() : Op(Operation::Propagate)
        {
            memset(Name, '\0', sizeof(Name));
        }

        ParticleOperation(unsigned long long int index, const string& name) : 
            Index(index), 
            Op(Operation::Propagate)
        {
            memset(Name, '\0', sizeof(Name));
            memcpy(Name, name.c_str(), (name.length() < 19 ? name.length() : 19));
        }

        ParticleOperation(unsigned long long int index, const char name[20], int verticalVector, int horizontalVector, int gradient, int mass, int speed, ParticleType type) : 
            Index(index), 
            Op(Operation::Land),
            VerticalVector(verticalVector),
            HorizontalVector(horizontalVector),
            Gradient(gradient),
            Mass(mass),
            Speed(speed),
            Type(type)
        {
            memcpy(Name, name, 20);
        }

        ParticleOperation(unsigned long long int index, const string& name, int verticalVector, int horizontalVector, int gradient, int mass, int speed, ParticleType type) : 
            Index(index), 
            Op(Operation::Land),
            VerticalVector(verticalVector),
            HorizontalVector(horizontalVector),
            Gradient(gradient),
            Mass(mass),
            Speed(speed),
            Type(type)
        {
            memset(Name, '\0', sizeof(Name));
            memcpy(Name, name.c_str(), (name.length() < 19 ? name.length() : 19));
        }
    };
}
