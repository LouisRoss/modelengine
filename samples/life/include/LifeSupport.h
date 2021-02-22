#pragma once

#include <iostream>
#include <vector>

#include "nlohmann/json.hpp"

#include "ConfigurationRepository.h"
#include "ProcessCallback.h"

#include "LifeOperation.h"
#include "LifeModelCarrier.h"
#include "LifeRecord.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::cout;
    using std::vector;

    using nlohmann::json;

    using embeddedpenguins::core::neuron::model::ConfigurationRepository;
    using embeddedpenguins::modelengine::threads::ProcessCallback;

    class LifeSupport
    {
        LifeModelCarrier& modelCarrier_;
        const ConfigurationRepository& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };

    public:
        LifeModelCarrier& Model() { return modelCarrier_; }
        const ConfigurationRepository& Configuration() const { return configuration_; }

    public:
        LifeSupport(LifeModelCarrier& modelCarrier, const ConfigurationRepository& configuration) :
            modelCarrier_(modelCarrier),
            configuration_(configuration)
        {
            const json& modelJson = configuration_.Configuration()["Model"];
            if (!modelJson.is_null() && modelJson.contains("Dimensions"))
            {
                auto dimensionElement = modelJson["Dimensions"];
                if (dimensionElement.is_array())
                {
                    auto dimensionArray = dimensionElement.get<vector<int>>();
                    width_ = dimensionArray[0];
                    height_ = dimensionArray[1];
                }
            }
        }

        const unsigned long int Width() const { return width_; }
        const unsigned long int Height() const { return height_; }

        bool AllocateModel(unsigned long int modelSize = 0)
        {
            if (!CreateModel(modelSize))
            {
                cout << "Unable to create model of size " << modelSize << "\n";
                return false;
            }

            return true;
        }

        void InitializeModel()
        {
            LoadOptionalDimensions();
        }

        void MakeStopSignal(unsigned long long int centerCell, vector<unsigned long long int>& initializedCells)
        {
            initializedCells.push_back(centerCell - 1);
            initializedCells.push_back(centerCell);
            initializedCells.push_back(centerCell + 1);
        }

        void MakeGlider(unsigned long long int centerCell, vector<unsigned long long int>& initializedCells)
        {
            initializedCells.push_back(centerCell - 1);
            initializedCells.push_back(centerCell);
            initializedCells.push_back(centerCell + 1);
            initializedCells.push_back(centerCell + width_ + 1);
            initializedCells.push_back(centerCell + (width_ * 2));
        }

        void InitializeCells(vector<unsigned long long int>& initializedCells)
        {
            for (auto cellIndex : initializedCells)
            {
                modelCarrier_.Model[cellIndex].Alive = true;
            }
        }

        void SignalInitialCells(vector<unsigned long long int>& initializedCells, ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            for (auto cellIndex : initializedCells)
            {
                SignalAllSurroundingCellsToEvaluate(cellIndex, callback);
            }
        }

        void SignalAllSurroundingCellsToEvaluate(unsigned long long int cellIndex, ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            for (auto rowStep = cellIndex - width_; rowStep < cellIndex + width_ + 1; rowStep += width_)
            {
                for (auto step = rowStep - 1; step < rowStep + 2; step++)
                {
                    callback(LifeOperation(step, Operation::Evaluate));
                }
            }
        }

        unsigned short int GetSurroundingCount(unsigned short int surround)
        {
            // Brian Kernigan says this will count the surrounding bits, so I trust him :)
            unsigned short int count = 0; 
            while (surround != 0)
            { 
                surround &= (surround - 1); 
                count++; 
            } 

            return count;
        }

    private:
        //
        // Allocate memory for the model.
        // NOTE: Only to be called from the main process, not a load library.
        //
        bool CreateModel(unsigned long int modelSize)
        {
            auto size = modelSize;
            if (size == 0)
            {
                const json& modelJson = configuration_.Configuration()["Model"];
                if (!modelJson.is_null() && modelJson.contains("ModelSize"))
                {
                    const json& modelSizeJson = modelJson["ModelSize"];
                    if (modelSizeJson.is_number_unsigned())
                        size = modelSizeJson.get<unsigned int>();
                }
            }

            if (size == 0)
            {
                cout << "No model size configured or supplied, initializer cannot create model\n";
                modelCarrier_.Valid = false;
                return false;
            }

            modelCarrier_.Model.resize(size);

            return true;
        }

        void LoadOptionalDimensions()
        {
            // Override the dimension defaults if configured.
            const json& configuration = configuration_.Configuration();
            auto& modelSection = configuration["Model"];
            if (!modelSection.is_null() && modelSection.contains("Dimensions"))
            {
                auto dimensionElement = modelSection["Dimensions"];
                if (dimensionElement.is_array())
                {
                    auto dimensionArray = dimensionElement.get<std::vector<int>>();
                    width_ = dimensionArray[0];
                    height_ = dimensionArray[1];
                }
            }

            maxIndex_ = width_ * height_;
        }
    };
}
