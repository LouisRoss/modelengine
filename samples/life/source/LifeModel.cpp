#include <stdio.h>
#include <string>
#include <iostream>
#include <memory>
#include <chrono>

#include "ModelEngine.h"
#include "sdk/ModelRunner.h"
#include "nlohmann/json.hpp"

#include "LifeOperation.h"
#include "LifeImplementation.h"
#include "LifeNode.h"
#include "LifeRecord.h"

#include "KeyListener.h"

using std::cout;
using std::string;
using std::vector;
using std::chrono::nanoseconds;
using std::chrono::system_clock;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::ceil;

using nlohmann::json;

using embeddedpenguins::modelengine::ModelEngine;
using embeddedpenguins::modelengine::sdk::ModelRunner;

using embeddedpenguins::life::infrastructure::LifeOperation;
using embeddedpenguins::life::infrastructure::LifeImplementation;
using embeddedpenguins::life::infrastructure::LifeNode;
using embeddedpenguins::life::infrastructure::LifeRecord;
using embeddedpenguins::life::infrastructure::KeyListener;

//////////////////////////////////////////////////////////// CPU Code ////////////////////////////////////////////////////////////
//
std::string cls("\033[2J\033[H");
bool displayOn = true;

char PrintAndListenForQuit(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner);
void PrintLifeScan(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner);
void ParseArguments(int argc, char* argv[]);

void PrintCellSurround(const vector<LifeNode>& model, unsigned short int cellIndex)
{
#if false
    for (auto i = 0; i < 50 * 25; i++)
    {
        cout << "Neuron " << i << " synapses:";

        auto& neuron = model[i];
        for (auto connection : neuron.PostsynapticConnections)
        {
            cout << " " << connection.PostsynapticNeuron << "/" << connection.Synapse;
        }
        cout << '\n';
    }
#endif
}

void TestLife()
{
    constexpr const char* configurationFilePath = "/home/louis/source/bmtk/workspace/chapter04/sim_ch04/simulation_config.json";

}

///////////////////////////////////////////////////////////////////////////
//Main program entry.
//Run the brain map.
//
int main(int argc, char* argv[])
{
    //TestLife();

    ParseArguments(argc, argv);
    ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord> modelRunner(argc, argv);
    if (!modelRunner.Run())
    {
        cout << "Cannot run model, stopping\n";
        return 1;
    }

    PrintAndListenForQuit(modelRunner);

    modelRunner.WaitForQuit();
    return 0;
}

char PrintAndListenForQuit(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner)
{
    char c;
    {
        auto listener = KeyListener();

        bool quit {false};
        while (!quit)
        {
            if (displayOn) PrintLifeScan(modelRunner);
            quit = listener.Listen(50'000, c);
        }
    }

    cout << "Received keystroke " << c << ", quitting\n";
    return c;
}

void PrintLifeScan(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner)
{
    cout << cls;

    auto node = begin(modelRunner.GetModel());
    std::advance(node, ((1000 * 1000 / 2) - (1000 * 20) + (1000 / 2 - 25)));
    for (auto high = 25; high; --high)
    {
        for (auto wide = 50; wide; --wide)
        {
            cout << (node->Alive ? "*" : " ");
            node++;
        }
        cout << '\n';

        std::advance(node, 1000 - 50);
    }

    cout << "Iterations: " << modelRunner.GetModelEngine().GetIterations() << "  Total work: " << modelRunner.GetModelEngine().GetTotalWork() << "                 \n";
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