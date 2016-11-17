#ifndef _CPP_TIME_H_
#define _CPP_TIME_H_

#include <stdint.h>

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <time.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::pair;

// VS中实现gettimeofday
#if defined(_MSC_VER)
typedef int32_t __timezone__;
int gettimeofday(timeval *timev, __timezone__ *timez);
#endif

class CppTime
{
public:

    static const uint32_t DAY_PER_WEEK;

    static const uint32_t HOUR_PER_DAY;
    static const uint32_t HOUR_PER_WEEK;

    static const uint32_t MINUTE_PER_HOUR;
    static const uint32_t MINUTE_PER_DAY;
    static const uint32_t MINUTE_PER_WEEK;

    static const uint32_t SECOND_PER_MINUTE;
    static const uint32_t SECOND_PER_HOUR;
    static const uint32_t SECOND_PER_DAY;
    static const uint32_t SECOND_PER_WEEK;

    //************************************
    // Describe:  获得时间的字符串形式,包含本地时区信息
    // Parameter: time_t timet              time_t类型时间,默认当前时间
    // Parameter: const char * pTimeFormat  时间格式字符串
    // Returns:   std::string
    // Author:    moon
    //************************************
    static string GetTimeStr(time_t timet = time(NULL), const string &timeFormat = "%Y-%m-%d %H:%M:%S");

    /** 获得指定time_t对应的日期：[1,31]
     *
     * @param   time_t timet
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t GetCurrDay(time_t timet = time(NULL));

    /** 获得指定time_t对应的月份：[1,12]
     *
     * @param   time_t timet
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t GetCurrMonth(time_t timet = time(NULL));

    /** 获得指定time_t对应的年份[1900,2038]
     *
     * @param   time_t timet
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t GetCurrYear(time_t timet = time(NULL));

    /** 获得指定time_t对应的小时[0,23]
     *
     * @param   time_t timet
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t GetCurrHour(time_t timet = time(NULL));

    /** 获得指定time_t对应的分钟[0,59]
     *
     * @param   time_t timet
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t GetCurrMinute(time_t timet = time(NULL));

    /** 获得指定time_t对应的秒数[0,60]，其中60是闰秒
     *
     * @param   time_t timet
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t GetCurrSecond(time_t timet = time(NULL));

    /** 获得指定time_t对应的星期[0,6]，其中0代表周日，1-6代表周一到周六
    *
    * @param   time_t timet
    * @retval  uint32_t
    * @author  moontan
    */
    static uint32_t GetCurrDayOfWeek(time_t timet = time(NULL));

    //************************************
    // Describe:  从特定格式的字符串中获得时间,包含本地时区信息
    // Parameter: const string & timeStr    必须是"2013-05-06 13:50:00"的格式
    // Returns:   time_t
    // Author:    moon
    //************************************
    static time_t GetTimeFromStr(const string &timeStr);

    //************************************
    // Describe:  判断2个时间是否是同一天,也可以测试给定的时间是否是今天,包含本地时区信息
    // Parameter: time_t time1
    // Parameter: time_t time2
    // Returns:   bool
    // Author:    moon
    //************************************
    static bool IsSameDay(time_t time1, time_t time2 = time(NULL))
    {
        // 考虑到时区的影响,此处使用效率不是很高的方法
        return CppTime::GetTimeStr(time1, "%Y-%m-%d") == CppTime::GetTimeStr(time2, "%Y-%m-%d");
    }

    //************************************
    // Describe:  timeval转成uint64_t值
    // Parameter: const timeval & timev
    // Returns:   std::uint64_t
    // Author:    moon
    //************************************
    static uint64_t Timev2Uint(const timeval &timev)
    {
        return timev.tv_sec * 1000000 + timev.tv_usec;
    }

    //************************************
    // Describe:  获得当前时间的uint64_t值
    // Returns:   uint64_t
    // Author:    moontan
    //************************************
    static uint64_t GetUTime()
    {
        timeval timev;
        gettimeofday(&timev, NULL);
        return Timev2Uint(timev);
    }

    //************************************
    // Describe:  uint64_t值转成timeval值
    // Parameter: uint64_t timeUint
    // Returns:   timeval
    // Author:    moon
    //************************************
    static timeval Uint2Timev(uint64_t timeUint)
    {
        timeval timev = { 0, 0 };
        timev.tv_sec = static_cast<long>(timeUint / 1000000);
        timev.tv_usec = static_cast<long>(timeUint % 1000000);

        return timev;
    }

    //************************************
    // Describe:  timeval相减
    // Parameter: const timeval & timev1
    // Parameter: const timeval & timev2
    // Returns:   std::int64_t
    // Author:    moon
    //************************************
    static int64_t TimevDiff(const timeval &timev1, const timeval &timev2)
    {
        return Timev2Uint(timev1) - Timev2Uint(timev2);
    }

    //************************************
    // Describe:  timeval与微秒数相加得到新的timeval，timei可以为负值，结果即为相减
    // Parameter: const timeval & timev
    // Parameter: int64_t timei
    // Returns:   timeval
    // Author:    moon
    //************************************
    static timeval TimevAdd(const timeval &timev, int64_t timei)
    {
        return Uint2Timev(Timev2Uint(timev) + timei);
    }

    //************************************
    // Describe:  获得时间的字符串形式,包含本地时区信息,在末尾加上.[微妙]，如：2013-12-15 20:26:40.422773
    // Parameter: time_t timet              time_t类型时间,默认当前时间
    // Parameter: const char * pTimeFormat  时间格式字符串
    // Returns:   std::string
    // Author:    moon
    //************************************
    static string GetUTimeStr(timeval *pTimeval = NULL, const string &timeFormat = "%Y-%m-%d %H:%M:%S");
};

class CppShowTimer
{
public:
    timeval StartTime;                              // 起始时间
    vector <pair<timeval, string> > TimeRecorder;   // 时间记录器

    CppShowTimer();

    string Start(const string &msg = "");

    //************************************
    // Describe:  记录时间
    // Parameter: const string & msg
    // Returns:   std::string           返回当前记录的条目信息
    // Author:    moontan
    //************************************
    string Record(const string &msg = "");

    //************************************
    // Describe:  是否已经启动
    // Returns:   bool
    // Author:    moontan
    //************************************
    bool IsStart() const
    {
        return lastTime.tv_sec != 0;
    }

private:
    timeval lastTime;
};

#endif
