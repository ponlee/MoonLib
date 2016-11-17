#ifndef _CPP_ENUM_TYPE_H_
#define _CPP_ENUM_TYPE_H_

#include <stdint.h>

#include <string>
#include <iostream>

using std::string;
using std::ostream;
using std::istream;

#if 0
/* 子类示例 */
class Sample:public CppEnumType
{
public:
    enum EnumValue
    {
        // TODO1:添加所有的枚举
        ENUM_COUNT
    };

    virtual const string *EnumString() const
    {
        static const string enumString[] =
        {
            // TODO2:添加与枚举对应的字符串
        };

        return enumString;
    }

    // TODO3:下面的名称改成类名
    OTHER_ENUM_CLASS_CODE(Sample)
};
#endif

#define OTHER_ENUM_CLASS_CODE(className) className() : CppEnumType() {};\
    ~className(){};\
    className(uint32_t value) : CppEnumType(value) {};\
    className(EnumValue value) : CppEnumType(value) {};\
    className(const string &valueStr)\
    {\
        mValue = StrToIntValue(valueStr);\
    }\
    className(const char *valueStr)\
    {\
        mValue = StrToIntValue(valueStr);\
    }\
    virtual uint32_t EnumCount() const\
    {\
        return ENUM_COUNT;\
    }

/*
 * 具有值和字符串类型的类
 */
class CppEnumType
{
public:
    static const string INVALID_VALUE;

    CppEnumType(uint32_t value): mValue(value) {};
    CppEnumType(): mValue(-1) {};
    CppEnumType(const string &valueStr);
    CppEnumType(const char *valueStr);
    virtual ~CppEnumType();

    virtual uint32_t EnumCount() const = 0;
    virtual const string *EnumString() const = 0;

    uint32_t StrToIntValue(const string &valueStr) const;
    const string &ToString() const;
    const char *c_str() const;
    uint32_t Value() const
    {
        return mValue;
    }

    bool operator<(const CppEnumType &right) const
    {
        return mValue < right.mValue;
    }

    bool operator>(const CppEnumType &right) const
    {
        return right < *this;
    }

    bool operator==(const CppEnumType &right) const
    {
        return !(*this < right || right < *this);
    }

    bool operator==(const string &valueStr) const
    {
        return ToString() == valueStr;
    }

    bool operator==(const char *valueStr) const
    {
        return ToString() == valueStr;
    }

    bool operator==(uint32_t value) const
    {
        return mValue == value;
    }

    template<typename T>
    bool operator!=(const T &right) const
    {
        return !(*this == right);
    }

    friend ostream &operator<<(ostream &os, CppEnumType &right)
    {
        return os << right.ToString();
    }

    friend istream &operator>>(istream &is, CppEnumType &right)
    {
        return is >> right.mValue;
    }

protected:
    uint32_t mValue;
};

#endif
