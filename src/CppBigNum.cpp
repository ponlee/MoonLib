#include "CppBigNum.h"

#include "CppString.h"
#include "CppArray.h"

CppBigNum::CppBigNum() :numStr("0"), flag(true)
{
}

CppBigNum::CppBigNum(const string &num) : flag(true)
{
    GetNumFromStr(num);
}

CppBigNum::CppBigNum(const char *num) : flag(true)
{
    GetNumFromStr(num);
}

CppBigNum::CppBigNum(const uint64_t num) : numStr(CppString::ToString(num)), flag(true)
{

}

CppBigNum::CppBigNum(const int64_t num) : numStr(CppString::ToString(abs(num))), flag(num >= 0)
{

}

CppBigNum::CppBigNum(const int32_t num) : numStr(CppString::ToString(abs(num))), flag(num >= 0)
{

}

CppBigNum::CppBigNum(const uint32_t num) : numStr(CppString::ToString(num)), flag(true)
{

}

CppBigNum::~CppBigNum()
{
}

void CppBigNum::GetNumFromStr(const string &num)
{
    string result;
    string::size_type length = num.size();
    uint64_t startIndex = 0;                  // 起始转换位置,主要是为了排除前面的0和正负号

    for (string::size_type i = 0; i < length; ++i)
    {
        if (startIndex == i && (num[i] == '+' || num[i] == '-' || num[i] == '0'))
        {
            ++startIndex;
            if (num[i] == '-')
            {
                flag = !flag;
            }
            continue;
        }

        // 含有非数字，置为0
        if (!isdigit(num[i]))
        {
            flag = true;
            numStr = "0";
            return;
        }
    }

    // 输入串全是0或者全是符号："0000","+++000","--+00","---","+++"
    if (startIndex == length)
    {
        flag = true;
        numStr = "0";
        return;
    }

    // 提高效率
    if (startIndex == 0)
    {
        numStr = num;
    }
    else
    {
        numStr = num.substr(startIndex);
    }

    if (numStr == "0")
    {
        flag = true;
    }
}

CppBigNum CppBigNum::Negative() const
{
    return (flag ? "-" : "") + numStr;
}

CppBigNum CppBigNum::Sqrt() const
{
    // 负数，直接返回0
    if (*this <= 0)
    {
        return 0;
    }

    // 给出前面几个小值的解法
    static uint32_t SMALL_RESULT[] = { 0, 1, 1, 1, 2 };

    if (*this < ARRAY_SIZE(SMALL_RESULT))
    {
        return SMALL_RESULT[CppString::FromString<uint32_t>(numStr)];
    }

    // 牛顿法开方
    CppBigNum k(1);             // 可任取 
    CppBigNum k2 = k * k;
    while (true)
    {
        if (*this >= k2)
        {
            if (*this - k2 <= k + k + 1)
            {
                return k;
            }
        }
        else
        {
            if (k2 - *this <= k + k - 1)
            {
                return k - 1;
            }
        }

        k = (k + *this / k) / 2;
        k2 = k * k;
        // printf(" k=%s\n", k.Value().c_str());
    }
}

CppBigNum CppBigNum::Abs() const
{
    return CppBigNum(numStr);
}

bool CppBigNum::numStrLess(const string &numStr1, const string &numStr2)
{
    if (numStr1.size() < numStr2.size())
    {
        return true;
    }

    if (numStr1.size() > numStr2.size())
    {
        return false;
    }

    return numStr1 < numStr2;
}

string CppBigNum::Value() const
{
    return (flag ? "" : "-") + numStr;
}

const CppBigNum CppBigNum::operator+(const CppBigNum &right) const
{
    string result;
    int32_t currValue;              // 当前的值
    bool lastFlag = false;          // 是否有进位，进位只能为1
    bool notEnd;
    bool currFlag;

    // 符号相同，值相加
    if (flag == right.flag)
    {
        currFlag = flag;

        string::const_reverse_iterator it1 = this->numStr.rbegin();
        string::const_reverse_iterator it2 = right.numStr.rbegin();

        // 逆序计算，包含进位
        while (true)
        {
            currValue = 0;
            notEnd = false;

            // 有进位
            if (lastFlag)
            {
                ++currValue;
                notEnd = true;
                lastFlag = false;
            }

            if (it1 != this->numStr.rend())
            {
                currValue += *it1 - '0';
                notEnd = true;
                ++it1;
            }

            if (it2 != right.numStr.rend())
            {
                currValue += *it2 - '0';
                notEnd = true;
                ++it2;
            }

            if (!notEnd)
            {
                break;
            }

            if (currValue >= 10)
            {
                lastFlag = true;
                currValue -= 10;
            }

            result += currValue + '0';
        }
    }
    else
    {
        // 符号不同，大数减小数
        const string *pBigNumStr = NULL;
        const string *pSmallNumStr = NULL;
        if (numStrLess(numStr, right.numStr))
        {
            pBigNumStr = &right.numStr;
            pSmallNumStr = &numStr;
            currFlag = right.flag;
        }
        else if (numStr == right.numStr)
        {
            return 0;
        }
        else
        {
            pBigNumStr = &numStr;
            pSmallNumStr = &right.numStr;
            currFlag = flag;
        }

        string::const_reverse_iterator it1 = pBigNumStr->rbegin();
        string::const_reverse_iterator it2 = pSmallNumStr->rbegin();

        while (true)
        {
            currValue = 0;
            notEnd = false;

            // 有借位
            if (lastFlag)
            {
                --currValue;
                notEnd = true;
                lastFlag = false;
            }

            if (it1 != pBigNumStr->rend())
            {
                currValue += *it1 - '0';
                notEnd = true;
                ++it1;
            }

            if (it2 != pSmallNumStr->rend())
            {
                currValue -= *it2 - '0';
                notEnd = true;
                ++it2;
            }

            if (!notEnd)
            {
                break;
            }

            if (currValue < 0)
            {
                lastFlag = true;
                currValue += 10;
            }

            result += currValue + '0';
        }
    }

    return CppBigNum((currFlag ? "" : "-") + CppString::Reverse(result));
}

const CppBigNum CppBigNum::operator-(const CppBigNum &right) const
{
    return *this + right.Negative();
}

bool CppBigNum::operator==(const CppBigNum &right) const
{
    if (numStr == "0" && right.numStr == "0")
    {
        return true;
    }

    if (*this < right)
    {
        return false;
    }

    if (right < *this)
    {
        return false;
    }

    return true;
}

bool CppBigNum::operator!=(const CppBigNum &right) const
{
    return !(*this == right);
}

bool CppBigNum::operator<(const CppBigNum &right) const
{
    if (flag == right.flag)
    {
        if (flag)
        {
            // 均为正数
            return numStrLess(numStr, right.numStr);
        }
        else
        {
            // 均为负数
            return numStrLess(right.numStr, numStr);
        }
    }

    // 负号不同，左侧为正数，则左侧大，否则右侧大
    return !flag;
}

bool CppBigNum::operator<=(const CppBigNum &right) const
{
    return *this < right || *this == right;
}

bool CppBigNum::operator>(const CppBigNum &right) const
{
    return right < *this;
}

bool CppBigNum::operator>=(const CppBigNum &right) const
{
    return *this > right || *this == right;
}

CppBigNum CppBigNum::operator++()
{
    *this += 1;
    return *this;
}

CppBigNum CppBigNum::operator++(int)
{
    CppBigNum result(*this);
    *this += 1;

    return result;
}

CppBigNum CppBigNum::operator-=(const CppBigNum &right)
{
    *this = *this - right;
    return *this;
}

CppBigNum CppBigNum::operator/=(const CppBigNum &right)
{
    *this = *this / right;
    return *this;
}

CppBigNum CppBigNum::operator%=(const CppBigNum &right)
{
    *this = *this % right;
    return *this;
}

CppBigNum CppBigNum::operator%(const CppBigNum &right)
{
    CppBigNum divValue = *this / right;
    return *this - divValue * right;
}

CppBigNum CppBigNum::Reverse() const
{
    return CppBigNum((flag ? "" : "-") + CppString::Reverse(numStr));
}

CppBigNum CppBigNum::operator+=(const CppBigNum &right)
{
    *this = *this + right;

    return *this;
}

const CppBigNum CppBigNum::operator*(const CppBigNum &right) const
{
    CppBigNum bigNumResult;
    string result;                      // 第2个数的每一位与第1个数的乘积，算上进位值
    string::const_reverse_iterator it1;
    uint64_t currValue = 0;
    uint64_t lastFlag = 0;              // 上次计算的进位值，只能为1位
    uint64_t it2DigitNum;               // 第2个数每一位的数值
    string currValueMul;                // 进位后的0

    // 把2个数逆序，然后逐位相乘，保留进位
    for (string::const_reverse_iterator it2 = right.numStr.rbegin();
        it2 != right.numStr.rend(); ++it2)
    {
        it2DigitNum = *it2 - '0';
        for (it1 = this->numStr.rbegin(); it1 != this->numStr.rend(); ++it1)
        {
            // 计算乘积
            currValue = (*it1 - '0') * it2DigitNum;

            // 加上进位
            if (lastFlag != 0)
            {
                currValue += lastFlag;
                lastFlag = 0;
            }

            // 计算下一位进位
            if (currValue >= 10)
            {
                lastFlag = currValue / 10;
                currValue = currValue % 10;
            }

            // 加上结果
            result += currValue + '0';
        }

        // 剩余进位需要加上
        if (lastFlag != 0)
        {
            result += lastFlag + '0';
            lastFlag = 0;
        }

        // 加上进位值currValueMul,逆序并加到结果中
        bigNumResult += CppBigNum(CppString::Reverse(result) + currValueMul);
        currValueMul += "0";
        result.clear();
    }

    bigNumResult.flag = !(flag^right.flag);

    if (bigNumResult == "0")
    {
        bigNumResult.flag = true;
    }

    return bigNumResult;
}

CppBigNum CppBigNum::operator*=(const CppBigNum &right)
{
    *this = *this * right;
    return *this;
}

const CppBigNum CppBigNum::operator/(const CppBigNum &right) const
{
    if (right == 0)
    {
        return 0;
    }

    CppBigNum currNum;          // 当前被除数用于计算的部分
    CppBigNum result;           // 结果
    CppBigNum rightNoFlag(right);   // 除数的正数形式
    rightNoFlag.flag = true;

    for (string::const_iterator it = numStr.begin(); it != numStr.end(); ++it)
    {
        uint32_t currValue = 0;     // 当前的结果数字
        if (currNum < rightNoFlag)
        {
            currNum *= 10;
            currNum += *it - '0';
        }

        // 从1到9，逐步相加，直到找到比当前被除数部分大的值
        if (currNum >= rightNoFlag)
        {
            CppBigNum multiValue = rightNoFlag;
            for (currValue = 0; currValue <= 9; ++currValue)
            {
                // 可以除出来
                if (multiValue > currNum)
                {
                    currNum -= multiValue - rightNoFlag;
                    break;
                }

                multiValue += rightNoFlag;
            }
        }

        result *= 10;
        result += currValue;
    }

    result.flag = !(flag^right.flag);
    return result;
}

CppBigNum & CppBigNum::operator=(const CppBigNum &right)
{
    numStr = right.numStr;
    flag = right.flag;
    return *this;
}
