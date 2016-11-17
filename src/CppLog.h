#ifndef _CPP_LOG_H_
#define _CPP_LOG_H_

#include <stdint.h>
#include <string.h>

#include <fstream>
#include <string>
#include <ostream>

#include "CppTime.h"
#include "CppString.h"

#ifdef WIN32
#define __builtin_expect(x,b) x
#endif

#ifndef likely
#define likely(x)       __builtin_expect((x),1)
#endif

#ifndef unlikely
#define unlikely(x)     __builtin_expect((x),0)
#endif

#define CURR_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define FILE_LOCATION (std::string(CURR_FILENAME) + ":" + CppString::ToString(__LINE__))

// TODO：
// 如果日志量过大，需要缓存，采用定时写入或者定量写入，不能每次都打开文件写入再关闭
// 重要的日志，比如ERROR和WARNN级别的，必要的时候需要flush到磁盘

/* 为了标点符号的匹配,所有的THROW最后都不要加入标点符号,所有Log最后都要加入标点符号 */
#ifdef USE_CPP_LOG_MACRO
#define Log(cppLog, logLevel, format, ...) if(cppLog != NULL && (cppLog)->mLogLevel <= (logLevel)){(cppLog)->LogMsg(std::string("[") + CppTime::GetUTimeStr() + "]["+CppString::ReplaceStr(#logLevel,"CppLog::")+"]" + FILE_LOCATION + "|" + CppString::GetArgs((format), ##__VA_ARGS__));}
#define LOG_THROW(cppLog, logLevel, format, ...) {Log(cppLog, logLevel, format, ##__VA_ARGS__);throw;}

// 如果定义了CPP_LOG_INSTANCE指定CppLog实例,可以使用下面的简易接口,方法是在使用前定义
#ifndef CPP_LOG_INSTANCE
#define CPP_LOG_INSTANCE &cppLog
#endif
#define TRACE_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::TRACE, format, ##__VA_ARGS__)
#define DEBUG_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::DEBUG, format, ##__VA_ARGS__)
#define INFOR_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::INFOR, format, ##__VA_ARGS__)
#define WARNN_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_LOG(format, ...) Log((CPP_LOG_INSTANCE), CppLog::ERROR, format, ##__VA_ARGS__)

#define TRACE_ILOG(cppLog, format, ...) Log((cppLog), CppLog::TRACE, format, ##__VA_ARGS__)
#define DEBUG_ILOG(cppLog, format, ...) Log((cppLog), CppLog::DEBUG, format, ##__VA_ARGS__)
#define INFOR_ILOG(cppLog, format, ...) Log((cppLog), CppLog::INFOR, format, ##__VA_ARGS__)
#define WARNN_ILOG(cppLog, format, ...) Log((cppLog), CppLog::WARNN, format, ##__VA_ARGS__)
#define ERROR_ILOG(cppLog, format, ...) Log((cppLog), CppLog::ERROR, format, ##__VA_ARGS__)

#define CHECK_OP_F(cppLog, expr, ret, op, logLevel, format, ...) if (unlikely(!(expr))){Log(cppLog, logLevel, "Check [" #expr "] Failed,ret[%d]." format, ret, ##__VA_ARGS__);op;}
#define ERROR_OP_F(cppLog, ret, op, logLevel, format, ...) if (unlikely(ret)){Log(cppLog, logLevel, "ret[%d]." format, ret, ##__VA_ARGS__);op;}

#define CHECK_RETURN_F(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_F(cppLog, expr, ret, return ret, logLevel, format, ##__VA_ARGS__)
#define CHECK_RETURN(cppLog, expr, ret, logLevel) CHECK_RETURN_F(cppLog,expr, ret, logLevel, "")
#define ERROR_RETURN_F(cppLog, ret, logLevel, format, ...) ERROR_OP_F(cppLog, ret, return ret, logLevel, format, ##__VA_ARGS__)
#define ERROR_RETURN(cppLog, ret, logLevel) ERROR_RETURN_F(cppLog, ret, logLevel, "")

#define CHECK_RETURN_VOID_F(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_F(cppLog, expr, ret, return, logLevel, format, ##__VA_ARGS__)
#define CHECK_RETURN_VOID(cppLog, expr, ret, logLevel) CHECK_RETURN_VOID_F(cppLog, expr, ret, logLevel, "")
#define ERROR_RETURN_VOID_F(cppLog, ret, logLevel, format, ...) ERROR_OP_F(cppLog, ret, return, logLevel, format, ##__VA_ARGS__)
#define ERROR_RETURN_VOID(cppLog, ret, logLevel) ERROR_RETURN_VOID_F(cppLog, ret, logLevel, "")

#define CHECK_BREAK_F(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_F(cppLog, expr, ret, break, logLevel, format, ##__VA_ARGS__)
#define CHECK_BREAK(cppLog, expr, ret, logLevel) CHECK_BREAK_F(cppLog,expr, ret, logLevel, "")
#define ERROR_BREAK_F(cppLog, ret, logLevel, format, ...) ERROR_OP_F(cppLog, ret, break, logLevel, format, ##__VA_ARGS__)
#define ERROR_BREAK(cppLog, ret, logLevel) ERROR_BREAK_F(cppLog, ret, logLevel, "")

#define CHECK_CONTINUE_F(cppLog, expr, ret, logLevel, format, ...) CHECK_OP_F(cppLog, expr, ret, continue, logLevel, format, ##__VA_ARGS__)
#define CHECK_CONTINUE(cppLog, expr, ret, logLevel) CHECK_CONTINUE_F(cppLog,expr, ret, logLevel, "")
#define ERROR_CONTINUE_F(cppLog, ret, logLevel, format, ...) ERROR_OP_F(cppLog, ret, continue, logLevel, format, ##__VA_ARGS__)
#define ERROR_CONTINUE(cppLog, ret, logLevel) ERROR_CONTINUE_F(cppLog, ret, logLevel, "")

#endif
#define THROW(format, ...) throw CppException(0, FILE_LOCATION + "|" + CppString::GetArgs((format), ##__VA_ARGS__))
#define THROW_CODE(code, format, ...) throw CppException((code), FILE_LOCATION + "|" + CppString::GetArgs((format), ##__VA_ARGS__))

#define CHECK_THROW_F(expr, format, ...) if (unlikely(!(expr)))THROW("Check [" #expr "] Failed." format, ##__VA_ARGS__)
#define CHECK_THROW(expr) CHECK_THROW_F(expr, "")
#define ERROR_THROW_F(ret, format, ...) if (unlikely(ret)){THROW("ret[%d]." format, ret, ##__VA_ARGS__);}
#define ERROR_THROW(ret) ERROR_THROW_F(ret, "")

class CppException
{
public:
    int32_t Code;
    std::string Msg;

    CppException(int32_t code = 0, const std::string &msg = "") : Code(code), Msg(msg)
    {
    }

    const std::string ToString(bool showZeroCode = false) const
    {
        if (showZeroCode || Code != 0)
        {
            return CppString::GetArgs("(%d)%s", Code, Msg.c_str());
        }

        return Msg;
    }

    friend ostream &operator<<(ostream &os, const CppException &right)
    {
        return os << right.ToString();
    }
};

class CppLog
{
public:
    enum LOG_LEVEL
    {
        TRACE,
        DEBUG,
        INFOR,
        WARNN,
        ERROR
    };

    CppLog(const std::string &logFile = "", LOG_LEVEL logLevel = TRACE, uint32_t maxFileSize = 0, uint32_t maxFileCount = 1);

    //************************************
    // Describe:  记录日志
    // Parameter: const std::string & msg
    // Returns:   void
    // Author:    moon
    //************************************
    void LogMsg(const std::string &msg);

    std::string mLogFile;           // 记录日志的文件
    LOG_LEVEL mLogLevel;            // 日志级别
    uint32_t mMaxFileSize;          // 日志文件最大大小,0表示不限制
    uint32_t mMaxFileCount;         // 日志文件最大数量,0表示不保存日志

    /** 获得调用栈
     *
     * @retval  string
     * @author  moontan
     */
    static string GetStackTrace();
};

#endif
