#pragma once

#include <string>

namespace test::embeddedpenguins::modelengine::infrastructure
{
    using std::string;

    class TestRecord
    {
        string row_;

    public:
        TestRecord(string& row) :
            row_(row)
        {

        }

        static const string Header()
        {
            return string("column1,column2,column3,column4");
        }

        const string Format()
        {
            return row_;
        }
    };
}
