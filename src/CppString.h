#ifndef _CPP_STRING_H_
#define _CPP_STRING_H_

#include <stdint.h>

#include <cstdarg>
#include <string>
#include <sstream>
#include <vector>
#include <set>

using namespace std;

class CppString
{
public:
    //************************************
    // Method:    Reverse
    // Access:    public static
    // Describe:  字符串逆序
    // Parameter: const string & srcString  要逆序的字符串
    // Returns:   std::string               逆序结果
    //************************************
    static string Reverse(const string &srcString);

    //************************************
    // Describe:  任意类型转换为string
    // Parameter: const T & value       任意类型
    // Parameter: int32_t divcision     小数位数,为-1表示不限制,默认为2
    // Returns:   const string
    // Author:    moon
    //************************************
    template <class T>
    static const string ToString(const T &value, int32_t divcision = 2);

    //************************************
    // Describe:  字符串转换为任意类型
    // Parameter: const string & value
    // Returns:   T
    // Author:    moon
    //************************************
    template <class T>
    static T FromString(const string &value);

    //************************************
    // Describe:  字符串分割
    // Parameter: string str                要分割的字符串
    // Parameter: const string & splitStr   分隔符。如果为空，result中返回原字符串
    // Parameter: vector<string> & result   保存分割结果
    // Parameter: bool removeEmptyElm       是否去除空串,默认去除
    // Parameter: size_t maxCount           最多分割段数,0表示不限制
    // Returns:   void
    // Author:    moon
    //************************************
    static void SplitStr(string str, const string &splitStr, vector<string> &result, bool removeEmptyElm = true, size_t maxCount = 0);
    static void SplitStr(string str, const vector<string> &splitStr, vector<string> &result, bool removeEmptyElm = true, size_t maxCount = 0);

    //************************************
    // Method:    ReplaceStr
    // FullName:  ReplaceStr
    // Describe:  字符串替换,将str中的oldValue全替换成newValue,不重复替换
    // Access:    public
    // Returns:   string
    // Qualifier:
    // Parameter: string str
    // Parameter: const string & oldValue
    // Parameter: const string & newValue
    //************************************
    static string ReplaceStr(string str, const string &oldValue, const string &newValue = "");

    //************************************
    // Describe:  去除指定字符串中左右括号包含的内容,如asd[123],将左右括号分别设置为'['和']',可以去除[123]
    // Parameter: const string & oldStr     要处理的字符串
    // Parameter: const char leftChar       左括号
    // Parameter: const char rightChar      右括号
    // Returns:   string                    处理结果
    // Author:    moon
    //************************************
    static string RemoveAngle(string str, const char leftChar, const char rightChar);

    //************************************
    // Describe:  字符串连接
    // Parameter: const string str[]
    // Parameter: size_t n
    // Parameter: const string & splitCh
    // Returns:   std::string
    // Author:    moon
    //************************************
    static string GetList(const string str[], size_t n, const string &splitCh);
    static string GetList(const vector<string> &str, const string &splitCh);
    static string GetList(const set<string> &str, const string &splitCh);

    //************************************
    // Describe:  将字符串重复值剔除
    // Parameter: const string & str        要去重的字符串
    // Parameter: const string & splitCh    分隔符
    // Parameter: bool removeEmptyElm       是否去除空串,默认去除
    // Returns:   std::string
    // Author:    moon
    //************************************
    static string Unique(const string &str, const string &splitCh, bool removeEmptyElm = true);

    //************************************
    // Describe:  剔除字符串左边的整个字符串，注意vector<string>参数的版本，替换查找的循序与vector的顺序一致
    // Parameter: const string & str        要处理的字符串
    // Parameter: const string & leftStr    要删除的左边的字符串
    // Parameter: int32_t times             剔除字符串的次数,默认为1,-1表示一直替换下去直到不可替换
    // Returns:   string
    // Author:    moon
    //************************************
    static string TrimLeft(string str, const string &trimStr, int32_t times = 1);
    static string TrimLeft(string str, const vector<string> &trimStrs, int32_t times = 1);

    //************************************
    // Describe:  剔除字符串左边的不可打印字符
    // Parameter: const string &str
    // Parameter: int32_t times             剔除字符串的次数,默认为1,-1表示一直替换下去直到不可替换
    // Returns:   string
    // Author:    moon
    //************************************
    static string TrimLeft(const string &str, int32_t times = 1);

    //************************************
    // Describe:  剔除字符串右边的整个字符串
    // Parameter: const string & str        要处理的字符串
    // Parameter: const string & rightStr   要删除的右边的字符串
    // Parameter: int32_t times             剔除字符串的次数,默认为1,-1表示一直替换下去直到不可替换
    // Returns:   string
    // Author:    moon
    //************************************
    static string TrimRight(string str, const string &trimStr, int32_t times = 1);
    static string TrimRight(string str, const vector<string> &trimStrs, int32_t times = 1);

    //************************************
    // Describe:  剔除字符串右边的不可打印字符
    // Parameter: const string &str
    // Parameter: int32_t times             剔除字符串的次数,默认为1,-1表示一直替换下去直到不可替换
    // Returns:   string
    // Author:    moon
    //************************************
    static string TrimRight(const string &str, int32_t times = 1);

    //************************************
    // Describe:  剔除字符串两边的整个字符串
    // Parameter: const string & str        要处理的字符串
    // Parameter: const string & trimStr    要删除的两边的字符串
    // Parameter: int32_t times             剔除字符串的次数,默认为1,-1表示一直替换下去直到不可替换
    // Returns:   string
    // Author:    moon
    //************************************
    static string Trim(string str, const string &trimStr, int32_t times = 1);
    static string Trim(string str, const vector<string> &trimStrs, int32_t times = 1);

    //************************************
    // Describe:  剔除字符串两边的不可打印字符
    // Parameter: string str
    // Parameter: int32_t times             剔除字符串的次数,默认为1,-1表示一直替换下去直到不可替换
    // Returns:   string
    // Author:    moon
    //************************************
    static string Trim(string str, int32_t times = 1);

    //************************************
    // Describe:  将字符串中的大写字母转小写
    // Parameter: string str
    // Returns:   std::string
    // Author:    moontan
    //************************************
    static string ToLower(string str);

    //************************************
    // Describe:  将字符串中的小写字母转大写
    // Parameter: string str
    // Returns:   std::string
    // Author:    moontan
    //************************************
    static string ToUpper(string str);

    //************************************
    // Describe:  计算子字符串的数量
    // Parameter: string str            源字符串
    // Parameter: const string & subStr 子字符串
    // Returns:   uint32_t              返回源字符串中子字符串的数量，如果源字符串和子字符串有一个为空，返回0
    // Author:    moon
    //************************************
    static uint32_t SubstrCount(string str, const string &subStr);

    //************************************
    // Describe:  格式化输出
    // Parameter: const char *format
    // Parameter: ...
    // Returns:   string
    // Author:    moon
    //************************************
    static string GetArgs(const char *format, ...);

    //************************************
    // Describe:  将数字转换为16进制字符
    // Parameter: uint8_t c
    // Returns:   char
    // Author:    moontan
    //************************************
    static char Hex2Char(uint8_t c, bool upCase = false);

    //************************************
    // Describe:  将Buf转换为十六进制串
    // Parameter: const string & str
    // Parameter: bool upCase
    // Returns:   string
    // Author:    moontan
    //************************************
    static string Hex2Str(const string &str, bool upCase = false);

    /** UTF8编码的字符串转GB2312编码
     *
     * @param   const string & utf8Src
     * @param   string & gb2312Dst
     * @retval  int32_t
     * @author  moontan
     */
    static int32_t Utf8ToGb2312(const string &utf8Src, string &gb2312Dst);

    /** GB2312编码的字符串转UTF8编码
     *
     * @param   const string & gb2312Src
     * @param   string & utf8Dst
     * @retval  int32_t
     * @author  moontan
     */
    static int32_t Gb2312ToUtf8(const string &gb2312Src, string &utf8Dst);
};

template <class T>
const string CppString::ToString(const T &value, int32_t divcision)
{
    stringstream ss;
    if(divcision >= 0)
    {
        ss.setf(ios::fixed);
        ss.precision(divcision);
    }

    ss << value;

    return ss.str();
}

template <class T>
T CppString::FromString(const string &value)
{
    if(value.length() == 0)
    {
        return  0;
    }

    stringstream ss(value);
    T result;
    ss >> result;

    return result;
}

#endif
