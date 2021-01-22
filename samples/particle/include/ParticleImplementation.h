#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>

#include "nlohmann/json.hpp"

#include "WorkerThread.h"
#include "WorkItem.h"
#include "ProcessCallback.h"
#include "Recorder.h"
#include "Log.h"

#include "ParticleCommon.h"
#include "ParticleSupport.h"
#include "ParticleOperation.h"
#include "ParticleNode.h"
#include "ParticleModelCarrier.h"
#include "ParticleRecord.h"

namespace embeddedpenguins::particle::infrastructure
{
    using std::cout;
    using std::string;
    using std::memset;
    using std::memcpy;
    using std::vector;
    using std::make_tuple;
    using std::tuple;
    using std::abs;

    using nlohmann::json;

    using ::embeddedpenguins::modelengine::threads::WorkerThread;
    using ::embeddedpenguins::modelengine::WorkItem;
    using ::embeddedpenguins::modelengine::threads::ProcessCallback;
    using ::embeddedpenguins::modelengine::Recorder;
    using ::embeddedpenguins::modelengine::Log;

    //
    //
    class ParticleImplementation : public WorkerThread<ParticleOperation, ParticleImplementation, ParticleRecord>
    {
        int workerId_;
        ParticleModelCarrier carrier_;
        const json& configuration_;

        unsigned long int width_ { 100 };
        unsigned long int height_ { 100 };
        unsigned long long int maxIndex_ { };
        
    public:
        //
        // Recommended to not allow a default constructor.
        // Not required.
        //
        ParticleImplementation() = delete;

        //
        // Required constructor.
        // Allow the template library to pass in the model and configuration
        // to each worker thread that is created.
        //
        ParticleImplementation(int workerId, ParticleModelCarrier carrier, const json& configuration) :
            workerId_(workerId),
            carrier_(carrier),
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
        }

        //
        // Required Initialize method.  
        // One instance of this class on one thread will be called
        // here to initialize the model.
        // Do not use this method to initialize instances of this class.
        //
        void Initialize(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int tickNow, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
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
        void Process(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int tickNow, 
            typename vector<WorkItem<ParticleOperation>>::iterator begin, 
            typename vector<WorkItem<ParticleOperation>>::iterator end, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            for (auto work = begin; work != end; work++)
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
        void Finalize(Log& log, Recorder<ParticleRecord>& record, unsigned long long int tickNow)
        {
        }

    private:
        void ProcessWorkItem(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int tickNow, 
            const ParticleOperation& work, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            string particleName(work.Name);
            switch (work.Op)
            {
            case Operation::Propagate:
                ProcessPropagation(log, record, particleName, work.Index, callback);
                break;

            case Operation::Land:
                ProcessLanding(log, record, work.Index, particleName, work.VerticalVector, work.HorizontalVector, work.Gradient, work.Mass, work.Speed, work.Type, callback);
                break;

            default:
                break;
            }
        }

        //
        // Handle the 'Propagate' operation by picking up the existing position
        // and velocity of the particle at the specified cell, and advancing them.
        // Pass the advanced parameters on to a new landing operation at the
        // index of the new position.  Vacate this index afterward.
        //
        void ProcessPropagation(Log& log, Recorder<ParticleRecord>& record, 
            const string& name,
            unsigned long long int index, 
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            auto& particleNode = carrier_.Model[index];

            auto [nextIndex, nextVerticalVector, nextHorizontalVector] = NewPositionAndVelocity(log, record, index, particleNode.VerticalVector, particleNode.HorizontalVector, particleNode.Gradient, callback);

            record.Record(ParticleRecord(name, ParticleRecordType::Land, nextIndex, particleNode));
            callback(ParticleOperation(nextIndex, name, nextVerticalVector, nextHorizontalVector, particleNode.Gradient, particleNode.Mass, particleNode.Speed, particleNode.Type));

#ifndef NOLOG
            log.Logger() << '<' << name << "> " << "Particle propagating from (" << particleNode.Name << ") index " << index << "\n";
            log.Logit();
#endif
            memset(particleNode.Name, '\0', sizeof(particleNode.Name));
            particleNode.Occupied = false;
        }

        //
        // Process the 'Landing' operation by dropping the described particle
        // into the specified index.  If that index is already occupied, handle
        // the collision by reversing the vector of the incoming particle as if
        // the original occupant of the index were an immovable object.
        //
        void ProcessLanding(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            const string& name,
            int verticalVector,
            int horizontalVector,
            int gradient,
            int mass,
            int speed,
            ParticleType type,
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            auto& particleNode = carrier_.Model[index];

            if (particleNode.Occupied)
            {
                verticalVector = -1 * verticalVector;
                horizontalVector = -1 * horizontalVector;

                auto [nextIndex, nextVerticalVector, nextHorizontalVector] = NewPositionAndVelocity(log, record, index, verticalVector, horizontalVector, particleNode.Gradient, callback);

#ifndef NOLOG
                log.Logger() << '<' << name << "> " << "Particle collision at index " << index << " already occupied by <" << particleNode.Name << ">\n";
                log.Logit();
#endif
                callback(ParticleOperation(nextIndex, name, nextVerticalVector, nextHorizontalVector, gradient, mass, speed, type));
                return;
            }

            memset(particleNode.Name, '\0', sizeof(particleNode.Name));
            memcpy(particleNode.Name, name.c_str(), (name.length() < sizeof(particleNode.Name) - 1 ? name.length() : sizeof(particleNode.Name) - 1));
            particleNode.Occupied = true;
            particleNode.VerticalVector = verticalVector;
            particleNode.HorizontalVector = horizontalVector;
            particleNode.Gradient = gradient;
            particleNode.Mass = mass;
            particleNode.Speed = speed;
            particleNode.Type = type;

#ifndef NOLOG
            log.Logger() << '<' << particleNode.Name << "> " << "Particle landed at index " << index << "\n";
            log.Logit();
#endif
            record.Record(ParticleRecord(name, ParticleRecordType::Propagate, index, particleNode));
            callback(ParticleOperation(index, name), 10 - particleNode.Speed);
        }

        //
        // Given a particle's index and vector, advance to the index of the
        // next particle position, taking into account 'boucing' off the
        // edges of the simulation as if they were perfectly elastic walls.
        //
        tuple<unsigned long long int, int, int> NewPositionAndVelocity(Log& log, Recorder<ParticleRecord>& record, 
            unsigned long long int index, 
            int verticalVector,
            int horizontalVector,
            int& gradient,
            ProcessCallback<ParticleOperation, ParticleRecord>& callback)
        {
            // NOTE - stay away from mixed signed/unsigned comparisons by casting everything signed.
            int width = (int)width_;
            int height = (int)height_;
            long long int currentIndex = (long long int)index;

            auto [horizontalStep, verticalStep] = DoBresenhamAlgorithm(verticalVector, horizontalVector, gradient);

            int nextVerticalPosition = currentIndex / width;
            int nextVerticalVector = verticalVector;
            nextVerticalPosition += verticalStep;
            if (nextVerticalPosition >= height)
            {
                nextVerticalPosition = height - (nextVerticalPosition - height) - 1;
                nextVerticalVector *= -1;
            }
            else if (nextVerticalPosition < 0)
            {
                nextVerticalPosition = 0 - nextVerticalPosition;
                nextVerticalVector *= -1;
            }

            int nextHorizontalPosition = currentIndex % width;
            int nextHorizontalVector = horizontalVector;
            nextHorizontalPosition += horizontalStep;
            if (nextHorizontalPosition >= width)
            {
                nextHorizontalPosition = width - (nextHorizontalPosition - width) - 1;
                nextHorizontalVector *= -1;
            }
            else if (nextHorizontalPosition < 0)
            {
                nextHorizontalPosition = 0 - nextHorizontalPosition;
                nextHorizontalVector *= -1;
            }

            auto nextIndex = (unsigned long long int)nextVerticalPosition * width_ + (unsigned long long int)nextHorizontalPosition;

            return make_tuple(nextIndex, nextVerticalVector, nextHorizontalVector);
        }

        //
        // An implementation of the Bresenham algorithm to draw the straightest line possible
        // give the quantization effects imposed by a discrete grid.
        // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
        //
        // The current vertical/horizontal vector indicates the direction the particle is traveling
        // (ignore the length of the vector), and the gradient (also kept as state within each particle) 
        // indicates the current error above or below the 'ideal straight line' from the current position.
        //
        // Return a tuple indicating the horizontal/vertical deltas to get to the next position.
        // Both numbers may be -1, 0, or 1.  Also return an updated value for the gradient.
        //
        tuple<int, int> DoBresenhamAlgorithm(int verticalVector, int horizontalVector, int& gradient)
        {
            int xStep {};
            int yStep {};

            auto absHorizontalVector = abs(horizontalVector);
            auto absVerticalVector = abs(verticalVector);
            auto widerThanTall = (absHorizontalVector > absVerticalVector);

            // After renormalizing to the first quandrant, we zero in on the right octant with these references.
            int& baseStep = widerThanTall ? xStep : yStep;
            int& riseStep =  widerThanTall ? yStep : xStep;
            int absBaseVector = widerThanTall ? absHorizontalVector : absVerticalVector;
            int absRiseVector = widerThanTall ? absVerticalVector : absHorizontalVector;

            // This ugly code finds the correct pair of magnitudes [-1, 0, 1] to move in the current octant.
            int baseMagnitude {};
            int riseMagnitude {};

            if (verticalVector >= 0 && horizontalVector >= 0)
            {
                if (widerThanTall)
                {
                    baseMagnitude = 1;
                    riseMagnitude = 1;
                }
                else
                {
                    baseMagnitude = 1;
                    riseMagnitude = 1;
                }
            }
            else if (verticalVector >= 0 && horizontalVector < 0)
            {
                if (widerThanTall)
                {
                    baseMagnitude = -1;
                    riseMagnitude = 1;
                }
                else
                {
                    baseMagnitude = 1;
                    riseMagnitude = -1;
                }
            }
            else if (verticalVector < 0 && horizontalVector < 0)
            {
                if (widerThanTall)
                {
                    baseMagnitude = -1;
                    riseMagnitude = -1;
                }
                else
                {
                    baseMagnitude = -1;
                    riseMagnitude = -1;
                }
            }
            else
            {
                if (widerThanTall)
                {
                    baseMagnitude = 1;
                    riseMagnitude = -1;
                }
                else
                {
                    baseMagnitude = -1;
                    riseMagnitude = 1;
                }
            }

            // Once all the octant information is straight, the actual algorithm is simple.
            if (gradient > 0)
            {
                baseStep = baseMagnitude;
                riseStep = riseMagnitude;
                gradient = gradient + absRiseVector - absBaseVector;
            }
            else
            {
                baseStep = baseMagnitude;
                riseStep = 0;
                gradient = gradient + absRiseVector;
            }

            return make_tuple(xStep, yStep);
        }
    };
}
