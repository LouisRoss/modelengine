#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>

#include "nlohmann/json.hpp"

#include "WorkerThread.h"
#include "WorkItem.h"
#include "ProcessCallback.h"
#include "Recorder.h"
#include "Log.h"

#include "LifeOperation.h"
#include "LifeNode.h"
#include "LifeRecord.h"

//#define NOLOG
namespace embeddedpenguins::life::infrastructure
{
    using std::cout;
    using std::vector;
    using std::begin;
    using std::end;
    using std::unique;
    using std::distance;

    using nlohmann::json;

    using ::embeddedpenguins::modelengine::threads::WorkerThread;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;
    using ::embeddedpenguins::modelengine::Log;
    using ::embeddedpenguins::modelengine::Recorder;
    using ::embeddedpenguins::modelengine::WorkItem;

    //
    // The implementation of the algorithm for Conway's game of life.
    // Note the required methods and their signatures:
    // * Constructor
    // * Initialize
    // * Process
    // * Finalize
    //
    // There is no interface to enforce implementation of these required methods.
    // Instead, the compiler will tell you if the implementation is wrong.
    // Use of an interface implies runtime polymorphism, which is slower
    // than compile-time polymorphism as implemented here using templates.
    //
    class LifeImplementation : public WorkerThread<LifeOperation, LifeImplementation, LifeRecord>
    {
        int workerId_;
        vector<LifeNode>& model_;
        const json& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };

        // The rules of life are captured in this table.
        // The index to the table is an integer bit pattern
        // containing an encoding of the nine alive-or-dead
        // states of a given center cell and its eight
        // surrounding cells.
        bool rules_[512] { false };

    public:
        //
        // Recommended to not allow a default constructor.
        // Not required.
        //
        LifeImplementation() = delete;

        //
        // Required constructor.
        // Allow the template library to pass in the model and configuration
        // to each worker thread that is created.
        //
        LifeImplementation(int workerId, vector<LifeNode>& model, const json& configuration) :
            workerId_(workerId),
            model_(model),
            configuration_(configuration)
        {
            // Override the dimension defaults if configured.
            auto dimensionElement = configuration_["Model"]["Dimensions"];
            if (dimensionElement.is_array())
            {
                auto dimensionArray = dimensionElement.get<vector<int>>();
                width_ = dimensionArray[0];
                height_ = dimensionArray[1];
            }

            maxIndex_ = width_ * height_;
            GenerateRulesOfLife();
        }

        //
        // Required Initialize method.  
        // One instance of this class on one thread will be called
        // here to initialize the model.
        // Do not use this method to initialize instances of this class.
        //
        void Initialize(Log& log, Recorder<LifeRecord>& record, 
            unsigned long long int tickNow, 
            ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
        }

        //
        // Required Process method.
        // Work items are partitioned in such a way that a single thread
        // may write to the model at the index in each work item in a
        // thread-safe manner.
        // Process the work items described by the iterators in whatever
        // manner is appropriate to the model.
        //
        void Process(Log& log, Recorder<LifeRecord>& record, 
            unsigned long long int tickNow, 
            typename vector<WorkItem<LifeOperation>>::iterator begin, 
            typename vector<WorkItem<LifeOperation>>::iterator end, 
            ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            if (end - begin == 0) return;

            // The work items tend to explode exponentially if we process duplicate work items.
            // Here we reduce the work load to include exactly one instance of each index.
            vector<WorkItem<LifeOperation>> localWork(begin, end);
            auto endIt = unique(localWork.begin(), localWork.end(), 
                [](const WorkItem<LifeOperation>& first, const WorkItem<LifeOperation>& second)
                {
                    return first.Operator.Index == second.Operator.Index;
                });

            for (auto work = localWork.begin(); work != endIt; work++)
            {
                ProcessWorkItem(log, record, tickNow, work->Operator, callback);
            }
        }

        //
        // Required Finalize method.
        // One instance of this class on one thread will be called
        // here to clean up the model.
        // Do not use this method as a destructor for instances of this class.
        // Destructors are allowed -- use one instead.
        //
        void Finalize(Log& log, Recorder<LifeRecord>& record, unsigned long long int tickNow)
        {
        }

    private:
        void ProcessWorkItem(Log& log, Recorder<LifeRecord>& record, 
            unsigned long long int tickNow, 
            const LifeOperation& work, 
            ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            switch (work.Op)
            {
            // The cell at work.Index received a signal to evaluate itself.
            // Its results will go into cell.AliveNextTick for use in the next Propagate tick.
            case Operation::Evaluate:
                ProcessEvaluation(log, record, work.Index, callback);
                break;

            // The cell at work.Index received a signal to propagate itself: AliveNextTick -> Alive.
            case Operation::Propagate:
                ProcessPropagation(log, record, work.Index, callback);
                break;

            default:
                break;
            }
        }

        //
        // An Evaluate operation has been received for this cell.
        // Apply the rules of Conway's game of life to this cell.
        // If it changes this cell's state, record the fact for the
        // next tick and signal a propagate operation for this cell
        // in the next tick.
        //
        void ProcessEvaluation(Log& log, Recorder<LifeRecord>& record, 
            unsigned long long int cellIndex, 
            ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            auto aliveNextTick = ApplyRulesOfLife(cellIndex);
            auto& lifeNode = model_[cellIndex];

            if (lifeNode.Alive != aliveNextTick)
            {
                lifeNode.AliveNextTick = aliveNextTick;

#ifndef NOLOG
                log.Logger() << "Cell " << cellIndex << " evaluating to " << (lifeNode.AliveNextTick ? "alive" : "dead") << " for next propagation" << '\n';
                log.Logit();
#endif
                callback(LifeOperation(cellIndex, Operation::Propagate));
            }
        }

        //
        // A Propagate operation has been received for this cell.
        // Propagate the previous evaluation to the cell's current
        // state, then signal all surrounding cells to evaluate
        // in the next tick.
        //
        void ProcessPropagation(Log& log, Recorder<LifeRecord>& record, 
            unsigned long long int cellIndex, 
            ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            auto& lifeNode = model_[cellIndex];
 
#ifndef NOLOG
            log.Logger() << "Cell " << cellIndex << " propagating to " << (lifeNode.AliveNextTick ? "alive" : "dead") << '\n';
            log.Logit();
#endif
            lifeNode.Alive = lifeNode.AliveNextTick;
            record.Record(LifeRecord(LifeRecordType::Propagate, cellIndex, lifeNode));

            SignalAllSurroundingCellsToEvaluate(log, record, cellIndex, callback);
        }

        void SignalAllSurroundingCellsToEvaluate(Log& log, Recorder<LifeRecord>& record, 
            unsigned long long int cellIndex,
            ProcessCallback<LifeOperation, LifeRecord>& callback)
        {
            for (auto rowStep = cellIndex - width_; rowStep < cellIndex + width_ + 1; rowStep += width_)
            {
                for (auto step = rowStep - 1; step < rowStep + 2; step++)
                {
                    callback(LifeOperation(cellIndex, Operation::Evaluate));
                }
            }
        }

        void GenerateRulesOfLife()
        {
            for (unsigned short int pattern = 0; pattern < 512; pattern++)
            {
                auto centerAlive = (pattern & 0x10) != 0;
                auto surroundingCount = GetSurroundingCount(pattern & ~0x10);

                auto centerResult { false };
                if (surroundingCount < 2 || surroundingCount > 3) centerResult = false;
                else if (surroundingCount == 3) centerResult = true;
                else centerResult = centerAlive;

                rules_[pattern] = centerResult;
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

        bool ApplyRulesOfLife(unsigned long long int cellIndex)
        {
            unsigned short int surround {};
            unsigned short int bitmask { 0x1 };
            for (auto rowStep = cellIndex - width_; rowStep < cellIndex + width_ + 1; rowStep += width_)
            {
                for (auto step = rowStep - 1; step < rowStep + 2; step++)
                {
                    if (step < maxIndex_ && model_[step].Alive) surround |= bitmask;
                    bitmask <<= 1;
                }
            }

            return rules_[surround];
        }
    };
}
