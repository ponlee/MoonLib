#include "CppEnv.h"

#include <unistd.h>
#ifndef __CYGWIN__
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#include "CppLog.h"

using namespace std;

string CppEnv::GetExePath()
{
    char path[PATH_MAX];
    int count = readlink("/proc/self/exe", path, sizeof(path));
    if (count < 0 || count >= (int)sizeof(path))
    {
        THROW("Can't get exe name,readlink error");
    }

    path[count] = 0;
    return path;
}

string CppEnv::GetExeName()
{
    string path = CppEnv::GetExePath();
    size_t index = path.find_last_of("/");
    if (index == string::npos)
    {
        return path;
    }

    return path.substr(index + 1);
}
