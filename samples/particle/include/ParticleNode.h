#pragma once

#include "ParticleCommon.h"

namespace embeddedpenguins::particle::infrastructure
{
    struct ParticleNode
    {
        char Name[20];
        int HorizontalVector { 0 };
        int VerticalVector { 0 };
        int Gradient { 0 };
        int Speed { 0 };
        int Mass { 0 };
        bool Occupied { false };
        ParticleType Type { ParticleType::Photon };
    };
}
