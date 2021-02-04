#include <stdio.h>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <memory>
#include <chrono>

#include "ModelEngine.h"
#include "sdk/ModelRunner.h"
#include "nlohmann/json.hpp"

#include "ParticleOperation.h"
#include "ParticleImplementation.h"
#include "ParticleNode.h"
#include "ParticleRecord.h"
#include "ParticleModelCarrier.h"

#include "KeyListener.h"

using std::cout;
using std::string;
using std::vector;
using std::tuple;
using std::make_tuple;
using std::chrono::nanoseconds;
using std::chrono::system_clock;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::ceil;

using nlohmann::json;

using embeddedpenguins::modelengine::ModelEngine;
using embeddedpenguins::modelengine::sdk::ModelRunner;

using embeddedpenguins::particle::infrastructure::ParticleOperation;
using embeddedpenguins::particle::infrastructure::ParticleImplementation;
using embeddedpenguins::particle::infrastructure::ParticleNode;
using embeddedpenguins::particle::infrastructure::ParticleType;
using embeddedpenguins::particle::infrastructure::ParticleRecord;
using embeddedpenguins::particle::infrastructure::KeyListener;
using embeddedpenguins::particle::infrastructure::ParticleModelCarrier;

//////////////////////////////////////////////////////////// CPU Code ////////////////////////////////////////////////////////////
//
std::string cls("\033[2J\033[H");
bool displayOn = true;
unsigned long int width { 100 };
unsigned long int height { 100 };
unsigned long int centerWidth {};
unsigned long int centerHeight {};

char PrintAndListenForQuit(ModelRunner<ParticleOperation, ParticleImplementation, ParticleModelCarrier, ParticleRecord>& modelRunner, ParticleModelCarrier& carrier);
void PrintLifeScan(ModelRunner<ParticleOperation, ParticleImplementation, ParticleModelCarrier, ParticleRecord>& modelRunner, ParticleModelCarrier& carrier);
void ParseArguments(int argc, char* argv[]);

///////////////////////////////////////////////////////////////////////////
//Main program entry.
//Run the game of life model.
//
int main(int argc, char* argv[])
{
    ParseArguments(argc, argv);
    vector<ParticleNode> model;
    ModelRunner<ParticleOperation, ParticleImplementation, ParticleModelCarrier, ParticleRecord> modelRunner(argc, argv);
    ParticleModelCarrier carrier { .Model = model };

    auto& configuration = modelRunner.Configuration();
    auto dimensionElement = configuration["Model"]["Dimensions"];
    if (dimensionElement.is_array())
    {
        auto dimensionArray = dimensionElement.get<vector<int>>();
        width = dimensionArray[0];
        height = dimensionArray[1];
    }
    centerWidth = width / 2;
    centerHeight = height / 2;

    if (!modelRunner.Run(carrier))
    {
        cout << "Cannot run model, stopping\n";
        return 1;
    }

    PrintAndListenForQuit(modelRunner, carrier);

    modelRunner.WaitForQuit();
    return 0;
}

char PrintAndListenForQuit(ModelRunner<ParticleOperation, ParticleImplementation, ParticleModelCarrier, ParticleRecord>& modelRunner, ParticleModelCarrier& carrier)
{
    constexpr char KEY_UP = 'A';
    constexpr char KEY_DOWN = 'B';
    constexpr char KEY_LEFT = 'D';
    constexpr char KEY_RIGHT = 'C';

    char c {' '};
    {
        KeyListener listener;

        bool quit {false};
        while (!quit)
        {
            if (displayOn) PrintLifeScan(modelRunner, carrier);
            auto gotChar = listener.Listen(50'000, c);
            if (gotChar)
            {
                switch (c)
                {
                    case KEY_UP:
                        if (centerHeight > 0) centerHeight--;
                        break;

                    case KEY_DOWN:
                        centerHeight++;
                        break;

                    case KEY_LEFT:
                        if (centerWidth > 0) centerWidth--;
                        break;

                    case KEY_RIGHT:
                        centerWidth++;
                        break;

                    case '=':
                    case '+':
                    {
                        auto newPeriod = modelRunner.EnginePeriod() / 10;
                        if (newPeriod < microseconds(100)) newPeriod = microseconds(100);
                        modelRunner.EnginePeriod() = newPeriod;
                        break;
                    }

                    case '-':
                    {
                        auto newPeriod = modelRunner.EnginePeriod() * 10;
                        if (newPeriod > microseconds(10'000'000)) newPeriod = microseconds(10'000'000);
                        modelRunner.EnginePeriod() = newPeriod;
                        break;
                    }

                    case 'q':
                    case 'Q':
                        quit = true;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    cout << "Received keystroke " << c << ", quitting\n";
    return c;
}

void PrintLifeScan(ModelRunner<ParticleOperation, ParticleImplementation, ParticleModelCarrier, ParticleRecord>& modelRunner, ParticleModelCarrier& carrier)
{
    constexpr int windowWidth = 100;
    constexpr int windowHeight = 30;

    auto occupancy = std::count_if(carrier.Model.begin(), carrier.Model.end(), 
        [](const ParticleNode& node){ return node.Occupied; });

    cout << cls;

    auto node = begin(carrier.Model);
    if (centerHeight < windowHeight / 2) centerHeight = windowHeight / 2;
    if (centerHeight >= height - (windowHeight / 2)) centerHeight = height - (windowHeight / 2) - 1;
    if (centerWidth < windowWidth / 2) centerWidth = windowWidth / 2;
    if (centerWidth >= width - (windowWidth / 2)) centerWidth = width - (windowWidth / 2) - 1;

    std::advance(node, ((width * (centerHeight - (windowHeight / 2))) + centerWidth - (windowWidth / 2)));
    for (auto high = windowHeight; high; --high)
    {
        for (auto wide = windowWidth; wide; --wide)
        {
            if (!node->Occupied) cout << ' ';
            else
            {
                switch (node->Type)
                {
                case ParticleType::Neutron: cout << '*'; break;
                case ParticleType::Electron: cout << '.'; break;
                case ParticleType::Fermion: cout << '!'; break;
                case ParticleType::Gluon: cout << '@'; break;
                case ParticleType::Photon: cout << '+'; break;
                default: cout << '-'; break;
                }
            }
            node++;
        }
        cout << '\n';

        std::advance(node, width - windowWidth);
        if (node >= end(carrier.Model)) node = begin(carrier.Model);
    }

    cout
        <<  occupancy << ":(" << centerWidth << "," << centerHeight << ") "
        << " Tick: " << modelRunner.EnginePeriod().count() << " us "
        << "Iterations: " << modelRunner.GetModelEngine().GetIterations() 
        << "  Total work: " << modelRunner.GetModelEngine().GetTotalWork() 
        << "                 \n";

    cout << "Arrow keys to navigate       + and - keys control speed            q to quit\n";
}

void ParseArguments(int argc, char* argv[])
{
    for (auto i = 0; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-d" || arg == "--nodisplay")
        {
            displayOn = false;
            cout << "Found -d flag, turning display off \n";
        }
    }
}
