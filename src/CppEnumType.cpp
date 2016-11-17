#include "CppEnumType.h"

const string CppEnumType::INVALID_VALUE = "InvalidValue";

CppEnumType::CppEnumType(const string &valueStr)
{
    mValue = StrToIntValue(valueStr);
}

CppEnumType::CppEnumType(const char *valueStr)
{
    mValue = StrToIntValue(valueStr);
}

const string &CppEnumType::ToString() const
{
    if(mValue >= EnumCount())
    {
        return INVALID_VALUE;
    }

    return EnumString()[mValue];
}

uint32_t CppEnumType::StrToIntValue(const string &valueStr) const
{
    uint32_t enumCount = EnumCount();
    uint32_t value = 0;
    const string *enumString = EnumString();
    for(; value < enumCount; ++value)
    {
        if(valueStr == enumString[value])
        {
            break;
        }
    }

    return value;
}

CppEnumType::~CppEnumType()
{

}

const char *CppEnumType::c_str() const
{
    return ToString().c_str();
}
