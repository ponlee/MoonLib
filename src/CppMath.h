#ifndef _CPP_MATH_H_
#define _CPP_MATH_H_

#include <stdint.h>

#include <unordered_set>
#include <set>
#include <bitset>
#include <cmath>

using std::unordered_set;
using std::set;
using std::bitset;

class CppMath
{
public:
    CppMath(void);
    ~CppMath(void);

    //************************************
    // Method:    IsInt
    // Access:    public
    // Describe:  判断double值在epsilon的范围内是否很接近整数
    //            如1.00005在epsilon为0.00005以上就很接近整数
    // Parameter: double doubleValue    要判断的double值
    // Parameter: double epsilon        判断的精度,0 < epsilon < 0.5
    // Parameter: INT32 & intValue      如果接近,返回最接近的整数值
    // Returns:   bool                  接近返回true,否则返回false
    //************************************
    static bool IsInt(double doubleValue, double epsilon, int32_t &intValue);

    //************************************
    // Method:    Sign
    // Access:    public
    // Describe:  获取value的符号
    // Parameter: T value   要获取符号的值
    // Returns:   INT32     正数、0和负数分别返回1、0和-1
    //************************************
    template <typename T>
    static int32_t Sign(T value);

    const static uint32_t MIN_PRIMER;       // 最小的素数

    //************************************
    // Describe:  判断一个数是否是素数
    // Parameter: uint32_t num      要判断的数
    // Returns:   bool              是素数返回true,否则返回false
    // Author:    moontan
    //************************************
    static bool IsPrime(uint32_t num);

    //************************************
    // Method:    IsIntegerSquare
    // Access:    public static
    // Describe:  判断给定的数开平方后是否为整数
    // Parameter: UINT32 num
    // Returns:   bool
    //************************************
    static bool IsIntegerSquare(uint32_t num);

    //************************************
    // Method:    GetDiffPrimerFactorNum
    // Access:    public static
    // Describe:  获取num所有的不同质因数
    // Parameter: UINT32 num
    // Returns:   set<UINT32>
    //************************************
    static set<uint32_t> GetDiffPrimerFactorNum(uint32_t num);

    //************************************
    // Method:    GetDigitMap
    // Access:    public
    // Describe:  获取num包含的数字map
    // Parameter: UINT32 num
    // Parameter: UINT16 & digitMap 返回的num的数字map,为2进制
    // Returns:   bool              如果包含重复的数字,返回false,否则返回true
    //************************************
    static bool GetDigitMap(uint32_t num, uint16_t &digitMap);

    //************************************
    // Method:    IsSameDigitNum
    // Access:    public 
    // Describe:  判断N个数字是否都是由相同的数字组成,每个数字都必须包含不同的数字
    // Parameter: const UINT32 nums[]   N个数字组成的数组
    // Parameter: UINT32 numCount       数字个数
    // Returns:   bool
    //************************************
    static bool IsNumsHaveSameDigit(const uint32_t nums[], uint32_t numCount);

    //************************************
    // Method:    Factorial
    // Access:    public 
    // Describe:  获得n的阶乘
    // Parameter: UINT32 n
    // Returns:   UINT32
    //************************************
    static uint32_t Factorial(uint32_t n);

    //************************************
    // Method:    Combination
    // Access:    public 
    // Describe:  求(n,r)的组合
    // Parameter: UINT32 n
    // Parameter: UINT32 r
    // Returns:   UINT32
    //************************************
    static uint32_t Combination(uint32_t n, uint32_t r);

    /** 获得[begin,end)之间的随机数
     *
     * @param   uint32_t begin      
     * @param   uint32_t end        为0表示不设置范围
     * @retval  uint32_t
     * @author  moontan
     */
    static uint32_t Random(uint32_t begin = 0, uint32_t end = 0);

    /** 求2数的最大公约数
     *
     * @param 	uint32_t num1
     * @param 	uint32_t num2
     * @retval 	uint32_t        如果num1或者num2其中至少有一个为0，则返回0，否则返回两数的最大公约数
     * @author 	moontan
     */
    template<class NumberType>
    static NumberType GCD(NumberType num1, NumberType num2);
};

template<class NumberType>
NumberType CppMath::GCD(NumberType num1, NumberType num2)
{
    if (num1 == 0 || num2 == 0)
    {
        return 0;
    }

    // 相减法求最大公约数
    while (true)
    {
        if (num1 == num2)
        {
            return num1;
        }

        if (num1 > num2)
        {
            num1 = num1 - num2;
        }
        else
        {
            num2 = num2 - num1;
        }
    }
}

/* 带有缓存的素数判断，会将已经判定为素数的数放到PrimerySet中，缓存会导致内存增加，并且可能导致效率降低（首次运行时），只有在大量重复素数判断时才应该使用 */
class CppMathPrimeWithCache
{
public:
    bool IsPrime(uint32_t num);

protected:
    unordered_set<uint32_t> PrimeSet;
};

/* 带有预计算的素数判断，先计算某一范围内所有的素数，如果之后判定的值在此范围内，则使用缓存，否则使用原始方式，当N比较大的时候，初始化需要一定时间 */
template <uint32_t N>
class CppMathPrimeWithInit
{
public:
    CppMathPrimeWithInit();
    bool IsPrime(uint32_t num) const;

protected:
    unordered_set<uint32_t> PrimeSet;
};

template <uint32_t N>
bool CppMathPrimeWithInit<N>::IsPrime(uint32_t num) const
{
    if (num <= N)
    {
        return PrimeSet.find(num) != PrimeSet.end();
    }

    return CppMath::IsPrime(num);
}

template <uint32_t N>
CppMathPrimeWithInit<N>::CppMathPrimeWithInit()
{
    bitset<N> primerBitset;
    uint32_t i, j;
    uint32_t sqrtNum = sqrt(N);
    for (i = 2; i <= sqrtNum; ++i)
    {
        if (!primerBitset[i])
        {
            for (j = i * i; j <= N; j += i)
            {
                primerBitset[j] = true;
            }
        }
    }

    // 未标记为非素数的即为素数
    for (i = 2; i <= N; ++i)
    {
        if (!primerBitset[i])
        {
            PrimeSet.insert(i);
        }
    }
}

#endif
