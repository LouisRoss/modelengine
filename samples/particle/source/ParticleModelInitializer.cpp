#include <string>
#include <ostream>

#include "ParticleSupport.h"
#include "ParticleModelInitializer.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::string;
    using std::ostringstream;

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
        int speed = 0;
        ParticleType type = ParticleType::Neutron;

        for (auto row = 0; row < height_; row += 8)
        {
            for (auto column = 0; column < width_; column += 8)
            {
                ostringstream nameStream;
                nameStream << "P(" << std::setw(3) << std::setfill('0') << row << ',' << std::setw(3) << std::setfill('0') << column << ')';
                support_.InitializeCell(model_, nameStream.str(), row, column, verticalVector, horizontalVector, mass, speed, type);

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
        support_.SignalInitialCells(model_, callback);
    }

    // the class factories

    extern "C" IModelInitializer<ParticleOperation, ParticleRecord>* create(vector<ParticleNode>& model, json& configuration) {
        return new ParticleModelInitializer(model, configuration);
    }

    extern "C" void destroy(IModelInitializer<ParticleOperation, ParticleRecord>* p) {
        delete p;
    }
}
