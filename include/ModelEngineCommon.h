#pragma once

#include "sys/stat.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "nlohmann/json.hpp"

namespace embeddedpenguins
{
    namespace modelengine::threads
    {
        enum class WorkCode
        {
            Run,
            Quit,
            Scan
        };
    }

    namespace modelengine
    {
        using std::cout;
        using std::string;
        using std::ifstream;
        using std::filesystem::create_directories;

        using nlohmann::json;
        
        enum class PartitionPolicy
        {
            ConstantWidth,
            AdaptiveWidth
        };
    }
}
