#ifndef _CPP_ENV_H_
#define _CPP_ENV_H_

#include "CppString.h"

using namespace std;

class CppEnv
{
public:
    static string GetExePath();
    static string GetExeName();
};

#endif
