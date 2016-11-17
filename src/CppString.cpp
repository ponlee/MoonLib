#include "CppString.h"

#include <iconv.h>
#include <alloca.h>

#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <cstdlib>

using namespace std;

string CppString::Reverse(const string &srcString)
{
    size_t len = srcString.length();
    string outString;

    for (size_t i = 0; i < len; ++i)
    {
        outString += srcString[len - i - 1];
    }

    return outString;
}

string CppString::ReplaceStr(string str, const string &oldValue, const string &newValue)
{
    string::size_type pos(0);

    while (true)
    {
        //查找词
        if ((pos = str.find(oldValue, pos)) != string::npos)
        {
            str.replace(pos, oldValue.length(), newValue);
            pos += newValue.length() - oldValue.length() + 1;
        }
        else
        {
            break;
        }
    }

    return str;
}

void CppString::SplitStr(string str, const string &splitStr, vector<string> &result, bool removeEmptyElm, size_t maxCount)
{
    SplitStr(str, vector<string>(1, splitStr), result, removeEmptyElm, maxCount);
}

void CppString::SplitStr(string str, const vector<string> &splitStr, vector<string> &result, bool removeEmptyElm, size_t maxCount)
{
    result.clear();
    size_t currCount = 0;          // 当前已获得段数

    // 从所有分割字符串中查找最小的索引
    size_t index = string::npos;
    size_t splitLen = 0;
    size_t currIndex;
    for (vector<string>::const_iterator it = splitStr.begin(); it != splitStr.end(); ++it)
    {
        if (it->length() == 0)
        {
            continue;
        }

        currIndex = str.find(*it);
        if (currIndex != string::npos && currIndex < index)
        {
            index = currIndex;
            splitLen = it->length();
        }
    }

    while (index != string::npos)
    {
        if (index != 0 || !removeEmptyElm)
        {
            // 将找到的字符串放入结果中
            ++currCount;
            if (maxCount > 0 && currCount >= maxCount)
            {
                break;
            }
            result.push_back(str.substr(0, index));
        }

        // 将之前的字符和分割符都删除
        str.erase(str.begin(), str.begin() + index + splitLen);

        // 继续查找下一个
        index = string::npos;
        for (vector<string>::const_iterator it = splitStr.begin(); it != splitStr.end(); ++it)
        {
            if (it->length() == 0)
            {
                continue;
            }

            currIndex = str.find(*it);
            if (currIndex != string::npos && currIndex < index)
            {
                index = currIndex;
                splitLen = it->length();
            }
        }
    }

    // 把剩下的放进去
    if (str.length() > 0 || !removeEmptyElm)
    {
        result.push_back(str);
    }
}

string CppString::RemoveAngle(string str, const char leftChar, const char rightChar)
{
    int startIndex = 0; // leftChar的位置
    int endIndex = 0;   // rightChar的位置

    while (true)
    {
        startIndex = str.find(leftChar);
        endIndex = str.find(rightChar, startIndex + 1);
        if (startIndex < 0 || endIndex < 0 || startIndex >= endIndex)
        {
            break;
        }

        str.erase(startIndex, endIndex - startIndex + 1);
    }

    return str;
}

string CppString::GetList(const string str[], size_t n, const string &splitCh)
{
    string result;

    if (n > 0)
    {
        result += str[0];
    }

    for (size_t i = 1; i < n; ++i)
    {
        result += splitCh + str[i];
    }

    return result;
}

string CppString::GetList(const vector<string> &str, const string &splitCh)
{
    string result;

    size_t n = str.size();

    if (n > 0)
    {
        result += str[0];
    }

    for (size_t i = 1; i < n; ++i)
    {
        result += splitCh + str[i];
    }

    return result;
}

string CppString::GetList(const set<string> &str, const string &splitCh)
{
    string result;

    size_t n = str.size();

    set<string>::const_iterator it = str.begin();

    if (n > 0)
    {
        result += *it++;
    }

    for (; it != str.end(); ++it)
    {
        result += splitCh + *it;
    }

    return result;
}

string CppString::Unique(const string &str, const string &splitCh, bool removeEmptyElm)
{
    vector<string> resultVec;
    CppString::SplitStr(str, splitCh, resultVec, removeEmptyElm);

    set<string> resultSet;
    resultSet.insert(resultVec.begin(), resultVec.end());

    return GetList(resultSet, splitCh);
}

string CppString::TrimLeft(string str, const string &trimStr, int32_t times /*= 1*/)
{
    while (times == -1 || times-- > 0)
    {
        if (str.find(trimStr) == 0)
        {
            str = str.substr(trimStr.length());
        }
        else
        {
            break;
        }
    }

    return str;
}

string CppString::TrimLeft(string str, const vector<string> &trimStrs, int32_t times /*= 1*/)
{
    bool notFound;

    while (times == -1 || times-- > 0)
    {
        notFound = true;

        for (vector<string>::const_iterator it = trimStrs.begin(); it != trimStrs.end(); ++it)
        {
            if (str.find(*it) == 0)
            {
                notFound = false;
                str = str.substr(it->length());
                break;
            }
        }

        if (notFound)
        {
            break;
        }
    }

    return str;
}

string CppString::TrimLeft(const string &str, int32_t times /*= 1*/)
{
    uint32_t i = 0;
    for (; i < str.size() && (times == -1 || static_cast<int32_t>(i) < times); ++i)
    {
        if (iswprint(str[i]) && !iswspace(str[i]))
        {
            break;
        }
    }

    return str.substr(i);
}

string CppString::TrimRight(string str, const string &trimStr, int32_t times /*= 1*/)
{
    string::size_type pos;

    while (times == -1 || times-- > 0)
    {
        pos = str.rfind(trimStr);
        if (pos != string::npos && pos + trimStr.length() == str.length())
        {
            str = str.substr(0, pos);
        }
        else
        {
            break;
        }
    }

    return str;
}

string CppString::TrimRight(string str, const vector<string> &trimStrs, int32_t times /*= 1*/)
{
    string::size_type pos;
    bool notFound;

    while (times == -1 || times-- > 0)
    {
        notFound = true;

        for (vector<string>::const_iterator it = trimStrs.begin(); it != trimStrs.end(); ++it)
        {
            pos = str.rfind(*it);
            if (pos != string::npos && pos + it->length() == str.length())
            {
                notFound = false;
                str = str.substr(0, pos);
                break;
            }
        }

        if (notFound)
        {
            break;
        }
    }

    return str;
}

string CppString::TrimRight(const string &str, int32_t times /*= 1*/)
{
    int32_t i = str.size() - 1;
    for (; i >= 0 && (times == -1 || static_cast<int32_t>(str.size() - 1 - i) < times); --i)
    {
        if (iswprint(str[i]) && !iswspace(str[i]))
        {
            break;
        }
    }

    return str.substr(0, i + 1);
}

string CppString::Trim(string str, const string &trimStr, int32_t times /*= 1*/)
{
    return TrimRight(TrimLeft(str, trimStr, times), trimStr, times);
}

string CppString::Trim(string str, const vector<string> &trimStrs, int32_t times /*= 1*/)
{
    return TrimRight(TrimLeft(str, trimStrs, times), trimStrs, times);
}

string CppString::Trim(string str, int32_t times /*= 1*/)
{
    return TrimRight(TrimLeft(str, times), times);
}

string CppString::ToLower(string str)
{
    for (string::iterator it = str.begin(); it != str.end(); ++it)
    {
        *it = tolower(*it);
    }

    return str;
}

string CppString::ToUpper(string str)
{
    for (string::iterator it = str.begin(); it != str.end(); ++it)
    {
        *it = toupper(*it);
    }

    return str;
}

uint32_t CppString::SubstrCount(string str, const string &subStr)
{
    uint32_t count = 0;
    string::size_type pos = 0;

    if (subStr.length() == 0 || str.length() == 0)
    {
        return 0;
    }

    while ((pos = str.find(subStr)) != string::npos)
    {
        str.erase(0, pos + 1);
        ++count;
    }

    return count;
}

string CppString::GetArgs(const char *format, ...)
{
    const size_t BUF_SIZE = 409600;

    va_list list;
    va_start(list, format);

    char buf[BUF_SIZE];
    vsnprintf(buf, sizeof(buf), format, list);
    va_end(list);

    return buf;
}

char CppString::Hex2Char(uint8_t c, bool upCase)
{
    c &= 0xf;
    if (c < 10)
    {
        return c + '0';
    }

    return c + (upCase ? 'A' : 'a') - 10;
}

string CppString::Hex2Str(const string &str, bool upCase /*= false*/)
{
    string result;
    string::size_type len = str.length();

    for (string::size_type i = 0; i < len; ++i)
    {
        result += Hex2Char(str[i] >> 4, upCase);
        result += Hex2Char(str[i], upCase);
    }

    return result;
}

int32_t CodeConv(iconv_t &cd, const string &src, string &dst)
{
    char *pSource = const_cast<char *>(src.data());
    size_t srcLen = src.length();
    size_t dstLen = srcLen * 1.5 + 1;
    size_t oldDstLen = dstLen;

    char *pDst = reinterpret_cast<char *>(alloca(dstLen));
    if (pDst == NULL)
    {
        return -3;
    }

    char *pOldPtr = pDst;            // 因为使用iconv后，地址会改变，所以要记录原始地址
    char **ppDest = reinterpret_cast<char **>(&pDst);

    int32_t ret = iconv(cd, &pSource, &srcLen, ppDest, &dstLen);
    iconv_close(cd);
    if (ret == 0)
    {
        dst.assign(pOldPtr, oldDstLen - dstLen);
    }

    return ret;
}

int32_t CppString::Utf8ToGb2312(const string &utf8Src, string &gb2312Dst)
{
    iconv_t cd;
    if ((cd = iconv_open("gb2312", "utf-8")) == 0)
    {
        return -2;
    }

    return CodeConv(cd, utf8Src, gb2312Dst);
}

int32_t CppString::Gb2312ToUtf8(const string &gb2312Src, string &utf8Dst)
{
    iconv_t cd;
    if ((cd = iconv_open("utf-8", "gb2312")) == 0)
    {
        return -2;
    }

    return CodeConv(cd, gb2312Src, utf8Dst);
}
