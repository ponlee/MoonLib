#ifndef _CPP_JSON_H_
#define _CPP_JSON_H_

#include <string>

#include <json/json.h>

#include "CppLog.h"

using std::string;

class CppJson
{
public:

    //************************************
    // Describe:  将字符串解析为Json::Value，解析失败会抛出异常
    // Parameter: const string & jsonStr
    // Returns:   Json::Value
    // Author:    moontan
    //************************************
    static Json::Value ParseJson(const string &jsonStr)throw(CppException);

    //************************************
    // Describe:  将Json::Value转为可读字符串
    // Parameter: const Json::Value & jsonValue
    // Returns:   string
    // Author:    moontan
    //************************************
    static string JsonToStyledStr(const Json::Value &jsonValue)
    {
        return jsonValue.toStyledString();
    }

    //************************************
    // Describe:  将Json::Value转为一行可读字符串
    // Parameter: const Json::Value & jsonValue
    // Returns:   string
    // Author:    moontan
    //************************************
    static string JsonToOneLineStr(const Json::Value &jsonValue)
    {
        Json::FastWriter fastwriter;
        return fastwriter.write(jsonValue);
    }
};

#endif
