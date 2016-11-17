#include "CppJson.h"

Json::Value CppJson::ParseJson(const string &jsonStr)throw(CppException)
{
    Json::Value jsonValue;
    Json::Reader jsonReader;
    if (!jsonReader.parse(jsonStr, jsonValue))
    {
        THROW("Json parse error!");
    }

    return jsonValue;
}
