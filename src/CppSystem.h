#ifndef _CPP_SYSTEM_H_
#define _CPP_SYSTEM_H_

#include <stdint.h>

#include <string>

#include "CppLog.h"

using namespace std;

class CppSystem
{
public:
    /** 执行命令，返回结果
     *
     * @param   const string & cmd
     * @retval  string
     * @author  moontan
     */
    static string ExcuteCommand(const string &cmd);
};

#endif
