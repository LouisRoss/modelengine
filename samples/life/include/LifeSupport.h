#pragma once

#include <iostream>
#include <vector>
#include <tuple>

#include "nlohmann/json.hpp"

#include "ProcessCallback.h"

#include "LifeOperation.h"
#include "LifeModelCarrier.h"
#include "LifeRecord.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::cout;
    using std::vector;
    using std::tuple;
    using std::make_tuple;

    using nlohmann::json;

    using embeddedpenguins::modelengine::threads::ProcessCallback;

    class LifeSupport
    {
        LifeModelCarrier modelCarrier_;
        json& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };

    public:
        LifeSupport(LifeModelCarrier modelCarrier, json& configuration) :
            modelCarrier_(modelCarrier),
            configuration_(configuration)
        {
            
        }

        const unsigned long int Width() const { return width_; }
        const unsigned long int Height() const { return height_; }

        void InitializeModel()
        {
            auto dimensionElement = configuration_["Model"]["Dimensions"];
            if (dimensionElement.is_array())
            {
                auto dimensionArray = dimensionElement.get<vector<int>>();
                width_ = dimensionArray[0];
                height_ = dimensionArray[1];
            }

            auto modelSize = width_ * height_;
            cout << "Using width = " << width_ << ", height = " << height_ << ", modelsize = " << modelSize << "\n";
            modelCarrier_.Model.resize(modelSize);
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
    };
}
