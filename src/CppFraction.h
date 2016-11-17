#ifndef _CPP_FRACTION_H_
#define _CPP_FRACTION_H_

#include <stdint.h>

#include <string>

#include <CppString.h>
#include <CppMath.h>

// 分数类
template<class NumberType>
class CppFraction
{
public:
    CppFraction();
    CppFraction(NumberType numerator, NumberType denominator, bool flag = true);

    /** 使用整数初始化分数
     *
     * @param 	int32_t numerator
     * @retval 	
     * @author 	moontan
     */
    CppFraction(int32_t num);

    bool Flag;              // 符号，1为正或者0，-1为负，0表示实例未初始化
    NumberType Numerator;     // 分子
    NumberType Denominator;   // 分母

    /** 分数加法
     *
     * @param 	const CppFraction<NumberType>& right
     * @retval 	CppFraction
     * @author 	moontan
     */
    CppFraction<NumberType>operator+(const CppFraction<NumberType>&right) const;

    /** 分数减法
     *
     * @param 	const CppFraction<NumberType>& right
     * @retval 	CppFraction
     * @author 	moontan
     */
    CppFraction<NumberType>operator-(const CppFraction<NumberType>&right) const;

    /** 分数乘法
     *
     * @param 	const CppFraction<NumberType>& right
     * @retval 	CppFraction
     * @author 	moontan
     */
    CppFraction<NumberType>operator*(const CppFraction<NumberType>&right) const;

    /** 分数除法
     *
     * @param 	const CppFraction<NumberType>& right
     * @retval 	CppFraction
     * @author 	moontan
     */
    CppFraction<NumberType>operator/(const CppFraction<NumberType>&right) const;

    /** 取负值
     *
     * @retval 	CppFraction
     * @author 	moontan
     */
    CppFraction<NumberType>Negative() const;

    CppFraction<NumberType>operator+=(const CppFraction<NumberType>&right);
    CppFraction<NumberType>operator-=(const CppFraction<NumberType>&right);
    CppFraction<NumberType>operator*=(const CppFraction<NumberType>&right);
    CppFraction<NumberType>operator/=(const CppFraction<NumberType>&right);

    /** 转换为可读字符串[分子]/[分母]
     *
     * @retval 	std::string
     * @author 	moontan
     */
    std::string ToString() const;

private:

};

template<class NumberType>
CppFraction<NumberType> CppFraction<NumberType>::operator/=(const CppFraction<NumberType>&right)
{
    *this = *this / right;
    return *this;
}

template<class NumberType>
CppFraction<NumberType> CppFraction<NumberType>::operator*=(const CppFraction<NumberType>&right)
{
    *this = *this * right;
    return *this;
}

template<class NumberType>
CppFraction<NumberType> CppFraction<NumberType>::operator-=(const CppFraction<NumberType>&right)
{
    *this = *this - right;
    return *this;
}

template<class NumberType>
CppFraction<NumberType> CppFraction<NumberType>::operator+=(const CppFraction<NumberType>&right)
{
    *this = *this + right;
    return *this;
}

template<class NumberType>
CppFraction<NumberType>::CppFraction(int32_t num) :Flag(num >= 0), Numerator(abs(num)), Denominator(1)
{
}


template<class NumberType>
CppFraction<NumberType>::CppFraction() :Flag(0), Numerator(0), Denominator(0)
{

}

template<class NumberType>
CppFraction<NumberType>CppFraction<NumberType>::operator+(const CppFraction<NumberType>&right) const
{
    NumberType numerator;
    bool flag;
    if (Flag == right.Flag)
    {
        numerator = Numerator * right.Denominator + right.Numerator * Denominator;
        flag = Flag;
    }
    else
    {
        NumberType leftNumerator = Numerator * right.Denominator;
        NumberType rightNumerator = right.Numerator * Denominator;
        if (leftNumerator > rightNumerator)
        {
            numerator = leftNumerator - rightNumerator;
            flag = Flag;
        }
        else
        {
            numerator = rightNumerator - leftNumerator;
            flag = right.Flag;
        }
    }

    NumberType denominator = Denominator * right.Denominator;

    return CppFraction(numerator, denominator, flag);
}

template<class NumberType>
CppFraction<NumberType>CppFraction<NumberType>::operator-(const CppFraction<NumberType>&right) const
{
    return *this + right.Negative();
}

template<class NumberType>
CppFraction<NumberType>CppFraction<NumberType>::operator*(const CppFraction<NumberType>&right) const
{
    return CppFraction(Numerator * right.Numerator, Denominator * right.Denominator, !(Flag^right.Flag));
}

template<class NumberType>
CppFraction<NumberType>CppFraction<NumberType>::operator/(const CppFraction<NumberType>&right) const
{
    return CppFraction(Numerator * right.Denominator, Denominator * right.Numerator, !(Flag^right.Flag));
}

template<class NumberType>
CppFraction<NumberType>CppFraction<NumberType>::Negative() const
{
    return CppFraction(Numerator, Denominator, !Flag);
}

template<class NumberType>
std::string CppFraction<NumberType>::ToString() const
{
    return CppString::ToString(Numerator) + "/" + CppString::ToString(Denominator);
}

template<class NumberType>
CppFraction<NumberType>::CppFraction(NumberType numerator, NumberType denominator, bool flag /*= true*/)
    : Flag(flag), Numerator(numerator), Denominator(denominator)
{
    // 把分子和分母都除以他们的最大公约数
    NumberType gcd = CppMath::GCD<NumberType>(Numerator, Denominator);
    if (gcd > 1)
    {
        Numerator /= gcd;
        Denominator /= gcd;
    }
}


#endif
