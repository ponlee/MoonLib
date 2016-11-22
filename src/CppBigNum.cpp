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

CppBigNum::CppBigNum(uint64_t num) : numStr(CppString::ToString(num)), flag(true)
{

}

CppBigNum::CppBigNum(int64_t num) : numStr(CppString::ToString(abs(num))), flag(num >= 0)
{

}

CppBigNum::CppBigNum(int32_t num) : numStr(CppString::ToString(abs(num))), flag(num >= 0)
{

}

CppBigNum::CppBigNum(uint32_t num) : numStr(CppString::ToString(num)), flag(true)
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
    static uint32_t SMALL_RESULT[] = {0, 1, 1, 1, 2};

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

    // 优化，如果小于32位最大值，转换为数字计算
    if (Abs() < uint32_t(-1) && right.Abs() < uint32_t(-1))
    {
        bigNumResult = CppString::FromString<uint64_t>(numStr) * CppString::FromString<uint64_t>(right.numStr);
    }
    else
    {
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

CppBigNumPro::CppBigNumPro() :flag(true)
{

}

CppBigNumPro::CppBigNumPro(const string &num) : flag(true)
{
    // TODO

}

CppBigNumPro::CppBigNumPro(const char *num) :flag(true)
{
    // TODO
}

CppBigNumPro::CppBigNumPro(uint64_t num) : flag(true)
{
    if (num == 0)
    {
        return;
    }

    numVec.push_back(num & 0xFFFFFFFF);
    uint32_t secondValue = num >> 32;
    if (secondValue > 0)
    {
        numVec.push_back(secondValue);
    }
}

CppBigNumPro::CppBigNumPro(int64_t num) :flag(num >= 0)
{
    if (num == 0)
    {
        return;
    }

    // 转换为正数
    if (num < 0)
    {
        num = -num;
    }

    numVec.push_back(num & 0xFFFFFFFF);
    uint32_t secondValue = num >> 32;
    if (secondValue > 0)
    {
        numVec.push_back(secondValue);
    }
}

CppBigNumPro::CppBigNumPro(int32_t num) :flag(num >= 0)
{
    if (num == 0)
    {
        return;
    }

    // 转换为正数
    if (num < 0)
    {
        num = -num;
    }

    numVec.push_back(num);
}

CppBigNumPro::CppBigNumPro(uint32_t num) :flag(true)
{
    numVec.push_back(num);
}

CppBigNumPro::CppBigNumPro(bool inFlag, vector<uint32_t> &&inNumVec) : flag(inFlag)
{
    numVec.swap(inNumVec);
}

CppBigNumPro::CppBigNumPro(bool inFlag, const vector<uint32_t> &inNumVec) : flag(inFlag)
{
    numVec = inNumVec;
}

CppBigNumPro::~CppBigNumPro()
{

}

string CppBigNumPro::Value() const
{
    CppBigNum bigNum;
    CppBigNum location(1);

    for (auto it = numVec.begin(); it != numVec.end(); ++it)
    {
        bigNum += location * (*it);
        location * (1 << 32);
    }

    return (flag ? "-" : "") + bigNum.Value();
}

const CppBigNumPro CppBigNumPro::operator+(const CppBigNumPro &right) const
{
    vector<uint32_t> result;
    int64_t currValue;              // 当前位计算的值，使用64位，不会溢出
    uint32_t lastValue = 0;         // 上一次计算的低位进位的值
    bool notEnd;
    bool resultFlag;

    // 符号相同，值相加
    if (flag == right.flag)
    {
        resultFlag = flag;

        vector<uint32_t>::const_iterator it1 = this->numVec.begin();
        vector<uint32_t>::const_iterator it2 = right.numVec.begin();

        // 逆序计算，包含进位
        while (true)
        {
            currValue = lastValue;
            notEnd = false;

            // 有进位
            if (lastValue != 0)
            {
                notEnd = true;
                lastValue = 0;
            }

            if (it1 != this->numVec.end())
            {
                currValue += *it1;
                notEnd = true;
                ++it1;
            }

            if (it2 != right.numVec.end())
            {
                currValue += *it2;
                notEnd = true;
                ++it2;
            }

            if (!notEnd)
            {
                break;
            }

            // 处理进位
            lastValue = currValue >> 32;
            if (lastValue != 0)
            {
                currValue &= 0xFFFFFFFF;
            }

            // 这里正常情况下不可能出现currValue == 0和lastValue同时为0并且计算结束的情况
            // 如果出现了，说明相加的2个数的数据结构哪里出问题了
            result.push_back(currValue);
        }
    }
    else
    {
        // 符号不同，大数减小数
        const vector<uint32_t> *pBigNumVec = NULL;
        const vector<uint32_t> *pSmallNumVec = NULL;
        if (numVecLess(numVec, right.numVec))
        {
            pBigNumVec = &right.numVec;
            pSmallNumVec = &numVec;
            resultFlag = right.flag;
        }
        else if (numVecLess(right.numVec, numVec))
        {
            pBigNumVec = &numVec;
            pSmallNumVec = &right.numVec;
            resultFlag = flag;
        }
        else
        {
            return 0;
        }

        vector<uint32_t>::const_iterator it1 = pBigNumVec->begin();
        vector<uint32_t>::const_iterator it2 = pSmallNumVec->begin();

        while (true)
        {
            currValue = 0;
            notEnd = false;

            // 有借位
            if (lastValue != 0)
            {
                currValue -= lastValue;
                notEnd = true;
                lastValue = 0;
            }

            if (it1 != pBigNumVec->end())
            {
                currValue += *it1;
                notEnd = true;
                ++it1;
            }

            if (it2 != pSmallNumVec->end())
            {
                currValue -= *it2;
                notEnd = true;
                ++it2;
            }

            if (!notEnd)
            {
                break;
            }

            if (currValue < 0)
            {
                lastValue = 1;
                currValue += (1 << 32);
            }

            result.push_back(currValue);
        }
    }

    return CppBigNumPro(resultFlag, std::move(result));
}

const CppBigNumPro CppBigNumPro::operator-(const CppBigNumPro &right) const
{
    return *this + right.Negative();
}

const CppBigNumPro CppBigNumPro::operator*(const CppBigNumPro &right) const
{
    CppBigNumPro bigNumResult;

    //     if (numVec.empty() || right.numVec.empty())
    //     {
    //         return 0;
    //     }
    // 
    //     // 优化，如果小于32位最大值，转换为64位数字计算
    //     if (numVec.size() == 1 && right.numVec.size() == 1)
    //     {
    //         bigNumResult = static_cast<uint64_t>(*numVec.begin()) * (*right.numVec.begin());
    //     }
    //     else
    //     {
    //         CppBigNumPro result;                        // 第2个数的每一位与第1个数的乘积，算上进位值
    //         vector<uint32_t>::const_iterator it1;
    //         uint64_t currValue = 0;
    //         uint64_t lastValue = 0;                     // 上次计算的进位值
    // 
    //         // 把2个数逐位相乘，保留进位
    //         for (vector<uint32_t>::const_iterator it2 = right.numVec.begin();
    //              it2 != right.numVec.end(); ++it2)
    //         {
    //             // 在前面补it2-right.numVec.begin()个0，表示进位，此轮it2的乘积直接放到这个result后面
    //             result.numVec.assign(it2 - right.numVec.begin(), 0);
    //             for (it1 = this->numVec.begin(); it1 != this->numVec.end(); ++it1)
    //             {
    //                 // 计算乘积
    //                 currValue = static_cast<uint64_t>(*it1) * (*it2);
    // 
    //                 // 加上进位，这里不会超过64位的最大值
    //                 currValue += lastValue;
    // 
    //                 // 计算下一位进位
    //                 lastValue = currValue >> 32;
    //                 currValue &= 0xFFFFFFFF;
    // 
    //                 // 加上结果
    //                 result += currValue;
    //             }
    // 
    //             // 剩余进位需要加上
    //             if (lastValue != 0)
    //             {
    //                 result += lastValue + '0';
    //                 lastValue = 0;
    //             }
    // 
    //             // 加上进位值currValueMul,逆序并加到结果中
    //             bigNumResult += CppBigNumPro(CppString::Reverse(result) + currValueMul);
    //             currValueMul += "0";
    //             result.clear();
    //         }
    //     }
    // 
    //     bigNumResult.flag = !(flag^right.flag);
    // 
    //     if (bigNumResult == "0")
    //     {
    //         bigNumResult.flag = true;
    //     }
    // 
    return bigNumResult;
}

const CppBigNumPro CppBigNumPro::operator/(const CppBigNumPro &right) const
{
    // TODO
    return 0;
}

bool CppBigNumPro::operator<(const CppBigNumPro &right) const
{
    if (flag == right.flag)
    {
        if (flag)
        {
            // 均为正数
            return numVecLess(numVec, right.numVec);
        }
        else
        {
            // 均为负数
            return numVecLess(right.numVec, numVec);
        }
    }

    // 负号不同，左侧为正数，则左侧大，否则右侧大
    return !flag;
}

bool CppBigNumPro::operator<=(const CppBigNumPro &right) const
{
    return *this < right || *this == right;

}

bool CppBigNumPro::operator>(const CppBigNumPro &right) const
{
    return right < *this;
}

bool CppBigNumPro::operator>=(const CppBigNumPro &right) const
{
    return *this > right || *this == right;
}

CppBigNumPro CppBigNumPro::operator++()
{
    *this += 1;
    return *this;
}

CppBigNumPro CppBigNumPro::operator++(int)
{
    CppBigNumPro result(*this);
    *this += 1;

    return result;
}

CppBigNumPro CppBigNumPro::operator%=(const CppBigNumPro &right)
{
    *this = *this % right;
    return *this;
}

CppBigNumPro CppBigNumPro::operator%(const CppBigNumPro &right)
{
    CppBigNumPro divValue = *this / right;
    return *this - divValue * right;
}

void CppBigNumPro::GetNumFromStr(const string &num)
{
    // TODO
}

CppBigNumPro CppBigNumPro::Negative() const
{
    return CppBigNumPro(!flag, numVec);
}

CppBigNumPro CppBigNumPro::Sqrt() const
{
    // TODO
    return 0;
}

CppBigNumPro CppBigNumPro::Abs() const
{
    return CppBigNumPro(true, numVec);
}

bool CppBigNumPro::numVecLess(const vector<uint32_t> &numVec1, const vector<uint32_t> &numVec2)
{
    if (numVec1.size() < numVec2.size())
    {
        return true;
    }

    if (numVec1.size() > numVec2.size())
    {
        return false;
    }

    // 位数相同，从最高位到最低位逐位比较
    auto it1 = numVec1.rbegin();
    auto it2 = numVec2.rbegin();
    for (; it1 != numVec1.rend() && it2 != numVec2.rend(); ++it1, ++it2)
    {
        if (*it1 < *it2)
        {
            return true;
        }

        if (*it1 > *it2)
        {
            return false;
        }
    }

    // 相等也返回false
    return false;
}

CppBigNumPro CppBigNumPro::operator/=(const CppBigNumPro &right)
{
    *this = *this / right;
    return *this;
}

CppBigNumPro CppBigNumPro::operator*=(const CppBigNumPro &right)
{
    *this = *this * right;
    return *this;
}

CppBigNumPro CppBigNumPro::operator-=(const CppBigNumPro &right)
{
    *this = *this - right;
    return *this;
}

CppBigNumPro CppBigNumPro::operator+=(const CppBigNumPro &right)
{
    *this = *this + right;
    return *this;
}

CppBigNumPro & CppBigNumPro::operator=(const CppBigNumPro &right)
{
    numVec = right.numVec;
    flag = right.flag;
    return *this;
}

bool CppBigNumPro::operator!=(const CppBigNumPro &right) const
{
    return !(*this == right);
}

bool CppBigNumPro::operator==(const CppBigNumPro &right) const
{
    if (numVec.empty() && right.numVec.empty())
    {
        return true;
    }

    if (flag != right.flag)
    {
        return false;
    }

    auto it1 = numVec.begin();
    auto it2 = numVec.begin();
    for (; it1 != numVec.end() && it2 != numVec.end(); ++it1, ++it2)
    {
        if (*it1 != *it2)
        {
            return false;
        }
    }

    return it1 == numVec.end() && it2 == numVec.end();
}
