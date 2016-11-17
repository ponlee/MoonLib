#include <iostream>

#include "gtest/gtest.h"

#include <CppMath.h>
using namespace std;

TEST(CppMath, CppMathIsIntTest)
{
    int32_t n;

    // 向下取整
    EXPECT_TRUE(CppMath::IsInt(1.0, 0.0005, n));
    EXPECT_EQ(n , 1);

    EXPECT_TRUE(CppMath::IsInt(-1.0, 0.0005, n));
    EXPECT_EQ(n , -1);

    EXPECT_TRUE(CppMath::IsInt(1.0004, 0.0005, n));
    EXPECT_EQ(n , 1);

    EXPECT_TRUE(CppMath::IsInt(-1.0004, 0.0005, n));
    EXPECT_EQ(n , -1);

    // 如果刚好为整，不OK，这里Ubuntu和macOS不一致，之后再看看
    //EXPECT_FALSE(CppMath::IsInt(1.0005, 0.0005, n));
    //EXPECT_FALSE(CppMath::IsInt(-1.0005, 0.0005, n));

    EXPECT_FALSE(CppMath::IsInt(-1.0006, 0.0005, n));
    EXPECT_FALSE(CppMath::IsInt(1.0006, 0.0005, n));

    // 向上取整
    // 如果刚好为整，也OK
    EXPECT_TRUE(CppMath::IsInt(0.9995, 0.0005, n));
    EXPECT_EQ(n , 1);

    EXPECT_TRUE(CppMath::IsInt(-0.9995, 0.0005, n));
    EXPECT_EQ(n , -1);

    EXPECT_TRUE(CppMath::IsInt(0.9996, 0.0005, n));
    EXPECT_EQ(n , 1);

    EXPECT_TRUE(CppMath::IsInt(-0.9996, 0.0005, n));
    EXPECT_EQ(n , -1);

    EXPECT_FALSE(CppMath::IsInt(0.9994, 0.0005, n));
    EXPECT_FALSE(CppMath::IsInt(-0.9994, 0.0005, n));
}

TEST(CppMath, CppMathIsPrimerTest)
{
    EXPECT_FALSE(CppMath::IsPrime(0));
    EXPECT_FALSE(CppMath::IsPrime(1));
    EXPECT_TRUE(CppMath::IsPrime(2));
    EXPECT_TRUE(CppMath::IsPrime(3));
    EXPECT_FALSE(CppMath::IsPrime(4));
    EXPECT_TRUE(CppMath::IsPrime(5));
    EXPECT_FALSE(CppMath::IsPrime(6));
    EXPECT_TRUE(CppMath::IsPrime(7));
    EXPECT_FALSE(CppMath::IsPrime(8));
    EXPECT_FALSE(CppMath::IsPrime(9));
    EXPECT_FALSE(CppMath::IsPrime(10));
    EXPECT_TRUE(CppMath::IsPrime(11));
    EXPECT_FALSE(CppMath::IsPrime(12));
    EXPECT_TRUE(CppMath::IsPrime(13));
    EXPECT_FALSE(CppMath::IsPrime(56883));
    EXPECT_TRUE(CppMath::IsPrime(56993));
    EXPECT_TRUE(CppMath::IsPrime(48600037));

    EXPECT_FALSE(CppMath::IsPrime(0));
    EXPECT_FALSE(CppMath::IsPrime(1));
    EXPECT_TRUE(CppMath::IsPrime(2));
    EXPECT_TRUE(CppMath::IsPrime(3));
    EXPECT_FALSE(CppMath::IsPrime(4));
    EXPECT_TRUE(CppMath::IsPrime(5));
    EXPECT_FALSE(CppMath::IsPrime(6));
    EXPECT_TRUE(CppMath::IsPrime(7));
    EXPECT_FALSE(CppMath::IsPrime(8));
    EXPECT_FALSE(CppMath::IsPrime(9));
    EXPECT_FALSE(CppMath::IsPrime(10));
    EXPECT_TRUE(CppMath::IsPrime(11));
    EXPECT_FALSE(CppMath::IsPrime(12));
    EXPECT_TRUE(CppMath::IsPrime(13));
    EXPECT_FALSE(CppMath::IsPrime(56883));
    EXPECT_TRUE(CppMath::IsPrime(56993));
    EXPECT_TRUE(CppMath::IsPrime(48600037));

    EXPECT_FALSE(CppMath::IsPrime(0));
    EXPECT_FALSE(CppMath::IsPrime(1));
    EXPECT_TRUE(CppMath::IsPrime(2));
    EXPECT_TRUE(CppMath::IsPrime(3));
    EXPECT_FALSE(CppMath::IsPrime(4));
    EXPECT_TRUE(CppMath::IsPrime(5));
    EXPECT_FALSE(CppMath::IsPrime(6));
    EXPECT_TRUE(CppMath::IsPrime(7));
    EXPECT_FALSE(CppMath::IsPrime(8));
    EXPECT_FALSE(CppMath::IsPrime(9));
    EXPECT_FALSE(CppMath::IsPrime(10));
    EXPECT_TRUE(CppMath::IsPrime(11));
    EXPECT_FALSE(CppMath::IsPrime(12));
    EXPECT_TRUE(CppMath::IsPrime(13));
    EXPECT_FALSE(CppMath::IsPrime(56883));
    EXPECT_TRUE(CppMath::IsPrime(56993));
    EXPECT_TRUE(CppMath::IsPrime(48600037));
}

TEST(CppMath, CppMathIsIntegerSquareTest)
{
    EXPECT_TRUE(CppMath::IsIntegerSquare(0));
    EXPECT_TRUE(CppMath::IsIntegerSquare(1));
    EXPECT_FALSE(CppMath::IsIntegerSquare(2));
    EXPECT_FALSE(CppMath::IsIntegerSquare(3));
    EXPECT_TRUE(CppMath::IsIntegerSquare(4));
    EXPECT_FALSE(CppMath::IsIntegerSquare(5));
    EXPECT_FALSE(CppMath::IsIntegerSquare(6));
    EXPECT_FALSE(CppMath::IsIntegerSquare(7));
    EXPECT_FALSE(CppMath::IsIntegerSquare(8));

    for(uint32_t i = 0; i < 100; ++i)
    {
        EXPECT_TRUE(CppMath::IsIntegerSquare(i * i));
    }
}

TEST(CppMath, CppMathGetDiffPrimerFactorNumTest)
{
    set<uint32_t> factors;
    set<uint32_t> factorsCompare;

    factors = CppMath::GetDiffPrimerFactorNum(1);
    EXPECT_EQ(true, factors.empty());

    factors = CppMath::GetDiffPrimerFactorNum(7);
    EXPECT_EQ(true, factors.empty());

    factors = CppMath::GetDiffPrimerFactorNum(14);
    factorsCompare.clear();
    factorsCompare.insert(2);
    factorsCompare.insert(7);
    EXPECT_EQ(factors , factorsCompare);

    factors = CppMath::GetDiffPrimerFactorNum(646);
    factorsCompare.clear();
    factorsCompare.insert(2);
    factorsCompare.insert(17);
    factorsCompare.insert(19);
    EXPECT_EQ(factors , factorsCompare);

    factors = CppMath::GetDiffPrimerFactorNum(644);
    factorsCompare.clear();
    factorsCompare.insert(2);
    factorsCompare.insert(7);
    factorsCompare.insert(23);
    EXPECT_EQ(factors , factorsCompare);
}