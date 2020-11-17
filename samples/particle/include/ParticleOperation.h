#pragma once

#include "ParticleCommon.h"

namespace embeddedpenguins::particle::infrastructure
{
    struct ParticleOperation
    {
        unsigned long long int Index { 0 };
        Operation Op;
        int VerticalVector {};
        int HorizontalVector {};
        int Mass {};
        int Speed {};

        ParticleOperation() : Op(Operation::Propagate) { }
        ParticleOperation(unsigned long long int index) : 
            Index(index), 
            Op(Operation::Propagate)
        {
            
        }

        ParticleOperation(unsigned long long int index, int verticalVector, int horizontalVector, int mass, int speed) : 
            Index(index), 
            Op(Operation::Land),
            VerticalVector(verticalVector),
            HorizontalVector(horizontalVector),
            Mass(mass),
            Speed(speed)
        {
            
        }
    };
}
