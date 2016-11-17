#include "CppRegex.h"

#include <sys/types.h>
#include <sys/times.h>

#include <cstring>
#include <cstdio>
#include <iostream>

#include <pcre/pcre.h>

#include "CppLog.h"

using namespace std;

string CppRegex::Match(const string &str)
{
    vector<string> result;
    Matches(str, result, 1);

    if(!result.empty())
    {
        return result[0];
    }

    return "";
}

uint32_t CppRegex::Matches(const string &str, vector<string> &result, uint32_t maxCount)
{
    result.clear();

    const size_t NUM = 3;
    int ovector[NUM];

    int rc = 0;
    size_t currCount = 0;
    size_t currStartIndex = 0;      // 字符串起始索引
    while(currCount < maxCount)
    {
        // 返回值：匹配成功返回非负数,没有匹配返回负数
        rc = pcre_exec(reg,         // code, 输入参数,用pcre_compile编译好的正则表达结构的指针
                NULL,               // extra, 输入参数,用来向pcre_exec传一些额外的数据信息的结构的指针
                str.c_str(),        // subject, 输入参数,要被用来匹配的字符串
                str.size(),         // length, 输入参数, 要被用来匹配的字符串的长度
                currStartIndex,     // startoffset, 输入参数,用来指定subject从什么位置开始被匹配的偏移量
                0,                  // options, 输入参数, 用来指定匹配过程中的一些选项
                ovector,            // ovector, 输出参数,用来返回匹配位置偏移量的数组
                NUM);               // ovecsize, 输入参数, 用来返回匹配位置偏移量的数组的最大大小

        if(rc > 0)
        {
            currCount++;
            currStartIndex = ovector[1];
            result.push_back(str.substr(ovector[0], ovector[1] - ovector[0]));
        }
        else
        {
            break;
        }
    }

    return currCount;
}


CppRegex::CppRegex(const string &inRegPattern) throw(CppException): regPattern(inRegPattern)
{
    const char *error;
    int errOffset;

    reg = pcre_compile(regPattern.c_str(), 0, &error, &errOffset, NULL);
    if(reg == NULL)
    {
        THROW("pcre_compile failed!error:%s", error);
    }
}

CppRegex::~CppRegex()
{
    pcre_free(reg);
}
