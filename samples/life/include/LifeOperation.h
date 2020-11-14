#pragma once

#include "LifeCommon.h"

namespace embeddedpenguins::life::infrastructure
{
    struct LifeOperation
    {
        unsigned long long int Index {0};
        Operation Op;

        LifeOperation() : Op(Operation::Evaluate) { }
        LifeOperation(unsigned long long int index, Operation op) : 
            Index(index), 
            Op(op)
        {
            
        }
    };
}
