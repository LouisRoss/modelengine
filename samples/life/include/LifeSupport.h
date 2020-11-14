#pragma once

#include <vector>

#include "ProcessCallback.h"

#include "LifeCommon.h"
#include "LifeNode.h"
#include "LifeOperation.h"
#include "LifeRecord.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::vector;

    using ::embeddedpenguins::modelengine::threads::ProcessCallback;

    class LifeSupport
    {
        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };

    public:
        LifeSupport(unsigned long width, unsigned long height) :
            width_(width),
            height_(height)
        {

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

        void InitializeCells(vector<LifeNode>& model, vector<unsigned long long int>& initializedCells)
        {
            for (auto cellIndex : initializedCells)
            {
                model[cellIndex].Alive = true;
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
