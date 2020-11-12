#pragma once

#include <chrono>
#include <string>
#include <fstream>
#include <map>
#include <tuple>

namespace embeddedpenguins::modelengine
{
    using std::multimap;
    using std::pair;
    using std::tuple;
    using std::make_tuple;
    using std::string;
    using std::begin;
    using std::end;
    using time_point = std::chrono::high_resolution_clock::time_point;
    using Clock = std::chrono::high_resolution_clock;
    using std::ofstream;

    template<class RECORDTYPE>
    class Recorder
    {
        unsigned long long int& ticks_;
        multimap<unsigned long long int, tuple<time_point, RECORDTYPE>> records_;

    public:
        Recorder(unsigned long long int& ticks) :
            ticks_(ticks)
        {
        }

        void Record(const RECORDTYPE& record)
        {
            records_.insert(pair<unsigned long long int, tuple<time_point, RECORDTYPE>>(ticks_, make_tuple(Clock::now(), record)));
        }

        static multimap<unsigned long long int, tuple<time_point, RECORDTYPE>>& MergedRecords()
        {
            static multimap<unsigned long long int, tuple<time_point, RECORDTYPE>> mr;
            return mr;
        }

        static void Merge(Recorder& other)
        {
            MergedRecords().insert(begin(other.records_), end(other.records_));
        }

        static void Print(const char* file)
        {
            ofstream recordfile;
            recordfile.open(file);

            recordfile << "tick,time," << RECORDTYPE::Header() << "\n";

            for (auto& record : MergedRecords())
            {
                auto rawtimestamp = std::get<0>(record.second);
                std::time_t timestamp = std::chrono::system_clock::to_time_t(rawtimestamp);
                string s(30, '\0');
                std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
                string formattedTime(s.c_str());

                auto lastSecond = std::chrono::floor<std::chrono::seconds>(std::get<0>(record.second));
                auto fractionOfSecond = rawtimestamp - lastSecond;

                recordfile 
                    << record.first
                    << ","
                    << formattedTime
                    << "." 
                    << std::setfill('0') << std::setw(9) << fractionOfSecond.count() 
                    << "," 
                    << std::get<1>(record.second).Format()
                    << "\n";
            }
        }
    };
}
