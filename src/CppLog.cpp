#include "CppLog.h"

#include <unistd.h>
#ifndef __CYGWIN__
#include <execinfo.h>
#endif
#include <cxxabi.h>

#include <cstdlib>
#include <iostream>

#include "CppFile.h"
#include "CppArray.h"

using namespace std;

void CppLog::LogMsg(const string &msg)
{
    if (mLogFile.empty())
    {
        cout << msg << endl;
        return;
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    // 如果最大文件数量为0，则不记录
    if (mMaxFileCount == 0)
    {
        return;
    }

    pthread_mutex_lock(&lock);

    // 超过大小了，进行日志文件处理
    if (mMaxFileSize > 0 && CppFile::GetFileSize(mLogFile) >= (int32_t)mMaxFileSize)
    {
        string::size_type dotIndex = mLogFile.rfind('.');
        if (dotIndex == string::npos)
        {
            dotIndex = mLogFile.size();
        }

        // 删除最老的日志
        string dstFile(mLogFile);
        dstFile.insert(dotIndex, CppString::ToString(mMaxFileCount - 1));
        unlink(dstFile.c_str());

        // 循环日志更名
        for (int32_t i = mMaxFileCount - 2; i >= 1; --i)
        {
            string srcFile(mLogFile);
            srcFile.insert(dotIndex, CppString::ToString(i));

            rename(srcFile.c_str(), dstFile.c_str());
            dstFile = srcFile;
        }

        rename(mLogFile.c_str(), dstFile.c_str());
    }

    CppFile::AppendMsg(mLogFile, msg + "\n");

    pthread_mutex_unlock(&lock);
}

CppLog::CppLog(const string &logFile /*= "/tmp/log.txt"*/, LOG_LEVEL logLevel /*= DEBUG*/, uint32_t maxFileSize /*= 0*/, uint32_t maxFileCount /*= 1*/)
    : mLogFile(logFile), mLogLevel(logLevel), mMaxFileSize(maxFileSize), mMaxFileCount(maxFileCount)
{

}

string CppLog::GetStackTrace()
{
#ifndef __CYGWIN__
    static const uint32_t MAX_STACK_TRACE_SIZE = 50;
    void *mStackTrace[MAX_STACK_TRACE_SIZE];
    static uint32_t stackSize = backtrace(mStackTrace, ARRAY_SIZE(mStackTrace));
    if (stackSize == 0)
    {
        return "<No stack trace>\n";
    }

    char** strings = backtrace_symbols(mStackTrace, stackSize);

    // 仅在DEBUG模式下能获得调用栈，非DEBUG模式可能会为NULL
    if (strings == NULL)
    {
        return "<Unknown error: backtrace_symbols returned NULL>\n";
    }

    string result;
    for (size_t i = 0; i < stackSize; ++i)
    {
        string mangledName = strings[i];
        string::size_type begin = mangledName.find('(');
        string::size_type end = mangledName.find('+', begin);
        if (begin == string::npos || end == string::npos)
        {
            result += mangledName;
            result += '\n';
            continue;
        }

        ++begin;
        int status;
        char* s = abi::__cxa_demangle(mangledName.substr(begin, end - begin).c_str(), NULL, 0, &status);
        if (status != 0)
        {
            result += mangledName;
            result += '\n';
            continue;
        }

        // Ignore ExceptionBase::Init so the top frame is the
        // user's frame where this exception is thrown.
        //
        // Can't just ignore frame#0 because the compiler might
        // inline ExceptionBase::Init.
        result += mangledName.substr(0, begin);
        result += s;
        result += mangledName.substr(end);
        result += '\n';

        free(s);
    }

    free(strings);
    return result;
#else
    return "<No stack trace>\n";
#endif
}
