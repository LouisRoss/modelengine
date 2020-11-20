#pragma once

namespace embeddedpenguins::particle::infrastructure
{
    enum class Operation
    {
        Propagate,
        Land,
        Collide
    };

    enum class ParticleType
    {
        Neutron,
        Electron,
        Fermion,
        Gluon,
        Photon
    };
}
