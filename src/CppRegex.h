#ifndef _CPP_REGEX_H_
#define _CPP_REGEX_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "CppLog.h"

using std::string;
using std::vector;

typedef struct real_pcre pcre;

class CppRegex
{
public:
    //************************************
    // Describe:  配置正则表达式
    // Parameter: const string & inRegPattern   要使用的正则表达式
    // Returns:
    // Author:    Cpp
    //************************************
    CppRegex(const string &inRegPattern)throw(CppException);
    ~CppRegex();

    //************************************
    // Describe:  正则表达式匹配,返回第一个匹配的字符串,如果没有匹配到,返回""
    // Parameter: const string & str
    // Returns:   string
    // Author:    moon
    //************************************
    string Match(const string &str);

    uint32_t Matches(const string &str, vector<string> &result, uint32_t maxCount = (uint32_t)-1);

    string regPattern;

private:
    pcre *reg;
};

#endif
