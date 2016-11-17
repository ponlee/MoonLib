#include "CppTime.h"

#include <cstdio>
#include <cstring>

#include "CppString.h"

using namespace std;

const uint32_t CppTime::DAY_PER_WEEK = 7;

const uint32_t CppTime::HOUR_PER_DAY = 24;
const uint32_t CppTime::HOUR_PER_WEEK = CppTime::HOUR_PER_DAY * CppTime::DAY_PER_WEEK;

const uint32_t CppTime::MINUTE_PER_HOUR = 60;
const uint32_t CppTime::MINUTE_PER_DAY = CppTime::MINUTE_PER_HOUR * CppTime::HOUR_PER_DAY;
const uint32_t CppTime::MINUTE_PER_WEEK = CppTime::MINUTE_PER_HOUR * CppTime::HOUR_PER_WEEK;

const uint32_t CppTime::SECOND_PER_MINUTE = 60;
const uint32_t CppTime::SECOND_PER_HOUR = CppTime::SECOND_PER_MINUTE * CppTime::MINUTE_PER_HOUR;
const uint32_t CppTime::SECOND_PER_DAY = CppTime::SECOND_PER_MINUTE * CppTime::MINUTE_PER_DAY;
const uint32_t CppTime::SECOND_PER_WEEK = CppTime::SECOND_PER_MINUTE * CppTime::MINUTE_PER_WEEK;

#if defined(_MSC_VER) 
int gettimeofday(timeval *timev, __timezone__ *timez)
{
    // 暂不实现时区功能
    if (timez != NULL)
    {
        *timez = 0;
    }

    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  systemTime;
    FILETIME    fileTime;
    uint64_t    time;

    GetSystemTime(&systemTime);
    SystemTimeToFileTime(&systemTime, &fileTime);
    time = static_cast<uint64_t>(fileTime.dwLowDateTime);
    time += static_cast<uint64_t>(fileTime.dwHighDateTime) << 32;

    timev->tv_sec = static_cast<uint64_t>((time - EPOCH) / 10000000L);
    timev->tv_usec = static_cast<uint64_t>(systemTime.wMilliseconds * 1000);

    return 0;
}
#endif

string CppTime::GetTimeStr(time_t timet /*= time(NULL)*/, const string &timeFormat /*= "%Y-%m-%d %H:%M:%S"*/)
{
    const size_t MAX_STR_LEN = 256;

    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    char buf[MAX_STR_LEN];
    strftime(buf, sizeof(buf), timeFormat.c_str(), localtime_r(&timet, &timeStruct));
    return buf;
}

time_t CppTime::GetTimeFromStr(const string &timeStr)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    //     static vector<string> splitChars;
    //     if (splitChars.empty())
    //     {
    //         splitChars.push_back("-");
    //         splitChars.push_back("/");
    //         splitChars.push_back(" ");
    //         splitChars.push_back(":");
    //     }
    // 
    //     vector<string> splitResult;
    //     CppString::SplitStr(timeStr, splitChars, splitResult);
    //     if (splitResult.size() >=1)
    //     {
    //         timeStruct.tm_year=CppString::GetArgs()
    //     }

    stringstream ss(timeStr);
    char tmpChar;
    ss >> timeStruct.tm_year;
    ss >> tmpChar;
    ss >> timeStruct.tm_mon;
    ss >> tmpChar;
    ss >> timeStruct.tm_mday;
    ss >> tmpChar;
    ss >> timeStruct.tm_hour;
    ss >> tmpChar;
    ss >> timeStruct.tm_min;
    ss >> tmpChar;
    ss >> timeStruct.tm_sec;

    //     sscanf((char *)timeStr.c_str(), "%d-%d-%d %d:%d:%d", &timeStruct.tm_year, &timeStruct.tm_mon, &timeStruct.tm_mday,
    //            &timeStruct.tm_hour, &timeStruct.tm_min, &timeStruct.tm_sec);
    timeStruct.tm_year -= 1900;
    timeStruct.tm_mon--;

    return mktime(&timeStruct);
}

string CppTime::GetUTimeStr(timeval *pTimeval /*= NULL*/, const string &timeFormat /*= "%Y-%m-%d %H:%M:%S"*/)
{
    timeval timeVal;
    if (pTimeval == NULL)
    {
        pTimeval = &timeVal;
        gettimeofday(pTimeval, NULL);
    }

    return CppString::GetArgs("%s.%06u", GetTimeStr(pTimeval->tv_sec, timeFormat).c_str(), pTimeval->tv_usec);
}

uint32_t CppTime::GetCurrDay(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_mday;
}

uint32_t CppTime::GetCurrMonth(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_mon + 1;
}

uint32_t CppTime::GetCurrYear(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_year + 1900;
}

uint32_t CppTime::GetCurrHour(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_hour;
}

uint32_t CppTime::GetCurrMinute(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_min;
}

uint32_t CppTime::GetCurrSecond(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_sec;
}

uint32_t CppTime::GetCurrDayOfWeek(time_t timet)
{
    tm timeStruct;
    memset(&timeStruct, 0, sizeof(timeStruct));

    (void)localtime_r(&timet, &timeStruct);

    return timeStruct.tm_wday;
}

string CppShowTimer::Record(const string &msg)
{
    timeval currTime;
    gettimeofday(&currTime, NULL);
    TimeRecorder.push_back(pair<timeval, string>(currTime, msg));

    string resultMsg = CppString::GetArgs("[%s]%s: From start %llu us,from last %llu us",
                                          CppTime::GetUTimeStr(&currTime).c_str(), msg.c_str(),
                                          CppTime::TimevDiff(currTime, StartTime),
                                          CppTime::TimevDiff(currTime, lastTime));
    lastTime = currTime;

    return resultMsg;
}

string CppShowTimer::Start(const string &msg)
{
    gettimeofday(&StartTime, NULL);
    lastTime = StartTime;
    TimeRecorder.clear();
    TimeRecorder.push_back(pair<timeval, string>(lastTime, msg));

    return CppString::GetArgs("[%s]%s", CppTime::GetUTimeStr(&StartTime).c_str(), msg.c_str());
}

CppShowTimer::CppShowTimer()
{
    StartTime.tv_sec = 0;
    StartTime.tv_usec = 0;
    lastTime.tv_sec = 0;
    lastTime.tv_usec = 0;
}
