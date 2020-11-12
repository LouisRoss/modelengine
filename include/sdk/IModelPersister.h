#pragma once

#include <string>
#include <vector>

namespace embeddedpenguins::neuron::infrastructure::persistence
{
    using std::string;
    using std::vector;

    //
    //
    template<class NODETYPE>
    class IModelPersister
    {
    public:
        virtual ~IModelPersister() = default;

        virtual bool LoadConfiguration() = 0;
        virtual bool ReadModel(vector<NODETYPE>& model) = 0;
    };
}
