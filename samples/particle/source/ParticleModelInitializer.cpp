#include "ParticleSupport.h"
#include "ParticleModelInitializer.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::vector;

    using embeddedpenguins::modelengine::sdk::IModelInitializer;
    
    ParticleModelInitializer::ParticleModelInitializer(vector<ParticleNode>& model, json& configuration) :
        ModelInitializer(model, configuration)
    {
    }

    void ParticleModelInitializer::Initialize()
    {
        auto dimensionElement = configuration_["Model"]["Dimensions"];
        if (dimensionElement.is_array())
        {
            auto dimensionArray = dimensionElement.get<vector<int>>();
            width_ = dimensionArray[0];
            height_ = dimensionArray[1];
        }

        support_ = ParticleSupport(width_, height_);

        auto modelSize = width_ * height_;
        cout << "Using width = " << width_ << ", height = " << height_ << ", modelsize = " << modelSize << "\n";
        model_.resize(modelSize);

        auto centerCell = (width_ * height_ / 2) + (width_ / 2);

        int verticalVector = -3;
        int horizontalVector = -3;
        int mass = 5;
        int speed = -10;

        for (auto row = 0; row < height_; row += 8)
        {
            for (auto column = 0; column < width_; column += 8)
            {
                support_.InitializeCell(model_, row, column, verticalVector, horizontalVector, mass, speed);

                verticalVector++;
                if (verticalVector > 3)
                {
                    verticalVector = -3;
                    horizontalVector++;
                    if (horizontalVector > 3) horizontalVector = -3;
                }
                speed++;
                if (speed > 10) speed = -10;
            }
        }
    }

    void ParticleModelInitializer::InjectSignal(ProcessCallback<ParticleOperation, ParticleRecord>& callback)
    {
        support_.SignalInitialCells(callback);
    }

    // the class factories

    extern "C" IModelInitializer<ParticleOperation, ParticleRecord>* create(vector<ParticleNode>& model, json& configuration) {
        return new ParticleModelInitializer(model, configuration);
    }

    extern "C" void destroy(IModelInitializer<ParticleOperation, ParticleRecord>* p) {
        delete p;
    }
}
