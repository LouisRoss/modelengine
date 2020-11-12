#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <tuple>

namespace embeddedpenguins::modelengine
{
    using std::chrono::system_clock;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;
    using std::map;
    using std::pair;
    using std::tuple;
    using std::make_tuple;
    using std::string;
    using std::ostringstream;
    using std::ofstream;

    enum class LogLevel
    {
        None,
        Status,
        Diagnostic
    };

    class Log
    {
        int id_;
        map<high_resolution_clock::time_point, tuple<int, string>> messages_;
        ostringstream stream_ {};

    public:
        Log() : id_(0)
        {

        }

        void SetId(int id)
        {
            id_ = id;
        }

        ostringstream& Logger()
        {
            return stream_;
        }

        void Logit(ostringstream& str)
        {
            messages_.insert(pair<high_resolution_clock::time_point, tuple<int, string>>(high_resolution_clock::now(), make_tuple(id_, str.str())));
        }

        void Logit()
        {
            messages_.insert(pair<high_resolution_clock::time_point, tuple<int, string>>(high_resolution_clock::now(), make_tuple(id_, stream_.str())));
            stream_.clear();
            stream_.str("");
        }

        static map<high_resolution_clock::time_point, tuple<int, string>>& MergedMessages() { static map<high_resolution_clock::time_point, tuple<int, string>> mm; return mm; }
        static void Merge(Log& other)
        {
            MergedMessages().insert(other.messages_.begin(), other.messages_.end());
        }

        static void Print(const char* file)
        {
            ofstream logfile;
            logfile.open(file);
            for (auto& message : MergedMessages())
            {
                std::time_t timestamp = std::chrono::system_clock::to_time_t(message.first);
                string s(30, '\0');
                std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
                string formattedTime(s.c_str());

                auto lastSecond = std::chrono::floor<std::chrono::seconds>(message.first);
                auto fractionOfSecond = message.first - lastSecond;

                logfile 
                    << "[" 
                    << std::get<0>(message.second) 
                    << "] " 
                    << formattedTime
                    << "." 
                    << std::setfill('0') << std::setw(9) << fractionOfSecond.count() 
                    << ": " 
                    << std::get<1>(message.second);
            }
        }

        static string FormatTime(high_resolution_clock::time_point time)
        {
            std::time_t timestamp = std::chrono::system_clock::to_time_t(time);
            string s(30, '\0');
            std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
            string formattedTime(s.c_str());

            auto lastSecond = std::chrono::floor<std::chrono::seconds>(time);
            auto fractionOfSecond = time - lastSecond;

            ostringstream str;
            str << formattedTime << '.' << std::setfill('0') << std::setw(9) << fractionOfSecond.count();

            return str.str();
        }
    };
}
