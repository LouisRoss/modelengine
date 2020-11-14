#include <stdio.h>
#include <string>
#include <vector>
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
unsigned long int width { 100 };
unsigned long int height { 100 };
unsigned long int centerWidth {};
unsigned long int centerHeight {};

char PrintAndListenForQuit(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner);
void PrintLifeScan(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner);
void ParseArguments(int argc, char* argv[]);

///////////////////////////////////////////////////////////////////////////
//Main program entry.
//Run the brain map.
//
int main(int argc, char* argv[])
{
    ParseArguments(argc, argv);
    ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord> modelRunner(argc, argv);

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
            if (displayOn) PrintLifeScan(modelRunner);
            auto gotChar = listener.Listen(50'000, c);
            if (gotChar)
            {
                if (c == '[')
                {
                    listener.Listen(50, c);
                    switch (c)
                    {
                        case KEY_UP:
                            centerHeight--;
                            break;

                        case KEY_DOWN:
                            centerHeight++;
                            break;

                        case KEY_LEFT:
                            centerWidth--;
                            break;

                        case KEY_RIGHT:
                            centerWidth++;
                            break;

                        default:
                            break;
                    }
                }
                else
                {
                    switch (c)
                    {
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
    }

    cout << "Received keystroke " << c << ", quitting\n";
    return c;
}

void PrintLifeScan(ModelRunner<LifeNode, LifeOperation, LifeImplementation, LifeRecord>& modelRunner)
{
    constexpr int windowWidth = 100;
    constexpr int windowHeight = 30;

    cout << cls;

    auto node = begin(modelRunner.GetModel());
    std::advance(node, ((width * (centerHeight - (windowHeight / 2))) + centerWidth - (windowWidth / 2)));
    for (auto high = windowHeight; high; --high)
    {
        for (auto wide = windowWidth; wide; --wide)
        {
            cout << (node->Alive ? "*" : " ");
            node++;
        }
        cout << '\n';

        std::advance(node, width - windowWidth);
        if (node >= end(modelRunner.GetModel())) node = begin(modelRunner.GetModel());
    }

    cout
        << "(" << centerWidth << "," << centerHeight << ") "
        << "Iterations: " << modelRunner.GetModelEngine().GetIterations() 
        << "  Total work: " << modelRunner.GetModelEngine().GetTotalWork() 
        << "                 \n";
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