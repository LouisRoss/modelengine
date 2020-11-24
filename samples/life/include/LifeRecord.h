#pragma once

#include <string>
#include <sstream>

#include "LifeCommon.h"
#include "LifeNode.h"

namespace embeddedpenguins::life::infrastructure
{
    using std::string;
    using std::ostringstream;

    enum class LifeRecordType
    {
        Evaluation,
        Propagate
    };

    struct LifeRecord
    {
        LifeRecordType Type { LifeRecordType::Evaluation };
        unsigned long long int LifeIndex { };
        bool Alive { false };

        LifeRecord(LifeRecordType type, unsigned long long int lifeIndex, const LifeNode& lifeNode) :
            Type(type),
            LifeIndex(lifeIndex),
            Alive(lifeNode.Alive)
        {
        }

        static const string Header()
        {
            ostringstream header;
            header << "Life-Event-Type,Life-Index,Life-Alive,u-l,up,u-r,right,l-r,lower,l-l,left";
            return header.str();
        }

        const string Format()
        {
            ostringstream row;
            row << (int)Type << "," << LifeIndex << "," << Alive << ",";
            switch (Type)
            {
                case LifeRecordType::Evaluation:

                    break;

                default:
                    row << "N/A,N/A";
                    break;
            }

            return row.str();
        }
    };
}
