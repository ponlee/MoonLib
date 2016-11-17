#ifndef _CPP_CRYPTO_H_
#define _CPP_CRYPTO_H_

#include <string>

#include "CppLog.h"

using std::string;

class CppCrypto
{
public:
   
    //************************************
    // Describe:  计算MD5值
    // Parameter: const string & data   原数据
    // Returns:   string                返回MD5的可读字符串格式
    // Author:    moontan
    //************************************
    static string Md5(const string &data);

    //************************************
    // Describe:  计算文件的MD5值
    // Parameter: const string & path   文件路径
    // Returns:   string                返回MD5的可读字符串格式
    // Author:    moontan
    //************************************
    static string Md5File(const string &path)throw(CppException);
};

#endif
