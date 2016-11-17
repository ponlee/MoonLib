#include <assert.h>

#include <cmath>

#include <iostream>

#include "gtest/gtest.h"

#include <CppBigNum.h>
using namespace std;

TEST(CppBigNum, CppBigNumTestReverse)
{
    EXPECT_EQ(CppBigNum(2), CppBigNum(2000).Reverse());
    EXPECT_EQ(CppBigNum(321), CppBigNum(123).Reverse());
}

TEST(CppBigNum, CppBigNumTestCompare)
{
    EXPECT_TRUE(CppBigNum(2) > CppBigNum(-1));
    EXPECT_FALSE(CppBigNum(-2) > CppBigNum(-1));
    EXPECT_TRUE(CppBigNum(-2) > CppBigNum(-3));
    EXPECT_FALSE(CppBigNum(-2) > CppBigNum(1));
    EXPECT_TRUE(CppBigNum(2) > CppBigNum(1));
    EXPECT_FALSE(CppBigNum(2) > CppBigNum(2));
    EXPECT_FALSE(CppBigNum(2) > CppBigNum(3));

    EXPECT_TRUE(CppBigNum(2) < CppBigNum(3));
    EXPECT_TRUE(CppBigNum(2) < CppBigNum(5));
    EXPECT_TRUE(CppBigNum(-2) < CppBigNum(1));
    EXPECT_TRUE(CppBigNum(123) < CppBigNum(245));
    EXPECT_TRUE(CppBigNum(9) < CppBigNum(245));
}

TEST(CppBigNum, CppBigNumTestOperator)
{
    const int32_t RANGE = 100;
    for (int32_t i = -RANGE; i <= RANGE; ++i)
    {
        // printf("%d\n", i);
        ASSERT_EQ(CppBigNum(i).Negative(), -i);
        ASSERT_EQ(++CppBigNum(i), i + 1);

        if (i >= 0)
        {
            ASSERT_EQ(CppBigNum(i).Sqrt(), uint32_t(sqrt(i)));
        }

        for (int32_t j = -RANGE; j <= RANGE; ++j)
        {
            //    printf("%d, %d\n", i, j);
            ASSERT_EQ(CppBigNum(i) - CppBigNum(j), i - j);
            ASSERT_EQ(CppBigNum(i) + CppBigNum(j), i + j);
            ASSERT_EQ(CppBigNum(i) * CppBigNum(j), i * j);
            if (j != 0)
            {
                ASSERT_EQ(CppBigNum(i) / CppBigNum(j), i / j);
            }
        }
    }
}