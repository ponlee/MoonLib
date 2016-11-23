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
    EXPECT_EQ(CppBigNum(100) / CppBigNum(-101), 0);
    EXPECT_EQ((CppBigNum(100) / CppBigNum(-101)).Value(), "0");
    EXPECT_EQ((CppBigNum(100) + CppBigNum(-100)).Value(), "0");

    const int32_t RANGE = 100;
    for (int32_t i = -RANGE; i <= RANGE; ++i)
    {
        //printf("%d\n", i);
        EXPECT_EQ(CppBigNum(i).Negative(), CppBigNum(-i));
        EXPECT_EQ(++CppBigNum(i), i + 1);

        if (i >= 0)
        {
            EXPECT_EQ(CppBigNum(i).Sqrt(), uint32_t(sqrt(i)));
        }

        for (int32_t j = -RANGE; j <= RANGE; ++j)
        {
            //    printf("%d, %d\n", i, j);
            EXPECT_EQ(CppBigNum(i) - CppBigNum(j), i - j);
            EXPECT_EQ(CppBigNum(i) + CppBigNum(j), i + j);
            EXPECT_EQ(CppBigNum(i) * CppBigNum(j), i * j);
            if (j != 0)
            {
                EXPECT_EQ(CppBigNum(i) / CppBigNum(j), i / j);
            }
        }
    }

    for (CppBigNum a = "189627391243132112356412332112563341892313212136512743681123231126653419862436125131629874398124362311233211238964";
         a < CppBigNum("189627391243132112356412332112563341892313212136512743681123231126653419862436125131629874398124362311233211238965");
         ++a)
    {
        auto b = a * a;
        EXPECT_EQ(b / a, a);
        EXPECT_EQ(b.Sqrt(), a);
    }

    EXPECT_EQ(CppBigNum("189627391243132112356412332112563341892313212136512743681123231126653419862436125131629874398124362311233211238964")
              * CppBigNum("56124135641123231165243654126354113221386127489763524513223416231345657456988746451673241352323148962436"),
              "10642673427401890287959143328411391642189930927246419475009998895455345122848209235404866983967924237303456059052419544157437954862459572950100014649960672276296454977189535827118789145330347503954873558823630655556304");

}

TEST(CppBigNum, CppBigNumProTestCompare)
{
    EXPECT_TRUE(CppBigNumPro(2) > CppBigNumPro(-1));
    EXPECT_FALSE(CppBigNumPro(-2) > CppBigNumPro(-1));
    EXPECT_TRUE(CppBigNumPro(-2) > CppBigNumPro(-3));
    EXPECT_FALSE(CppBigNumPro(-2) > CppBigNumPro(1));
    EXPECT_TRUE(CppBigNumPro(2) > CppBigNumPro(1));
    EXPECT_FALSE(CppBigNumPro(2) > CppBigNumPro(2));
    EXPECT_FALSE(CppBigNumPro(2) > CppBigNumPro(3));

    EXPECT_TRUE(CppBigNumPro(2) < CppBigNumPro(3));
    EXPECT_TRUE(CppBigNumPro(2) < CppBigNumPro(5));
    EXPECT_TRUE(CppBigNumPro(-2) < CppBigNumPro(1));
    EXPECT_TRUE(CppBigNumPro(123) < CppBigNumPro(245));
    EXPECT_TRUE(CppBigNumPro(9) < CppBigNumPro(245));
}

TEST(CppBigNum, CppBigNumProTestOperator)
{
    EXPECT_EQ(CppBigNum(100) / CppBigNum(-101), 0);
    EXPECT_EQ((CppBigNum(100) / CppBigNum(-101)).Value(), "0");
    EXPECT_EQ((CppBigNum(100) + CppBigNum(-100)).Value(), "0");

    const int32_t RANGE = 100;
    for (int32_t i = -RANGE; i <= RANGE; ++i)
    {
        // printf("%d\n", i);
        EXPECT_EQ(CppBigNumPro(i).Negative(), -i);
        EXPECT_EQ(++CppBigNumPro(i), i + 1);

        if (i >= 0)
        {
            EXPECT_EQ(CppBigNumPro(i).Sqrt(), uint32_t(sqrt(i)));
        }

        for (int32_t j = -RANGE; j <= RANGE; ++j)
        {
            // printf("%d, %d\n", i, j);
            EXPECT_EQ(CppBigNumPro(i) - CppBigNumPro(j), i - j);
            EXPECT_EQ(CppBigNumPro(i) + CppBigNumPro(j), i + j);
            EXPECT_EQ(CppBigNumPro(i) * CppBigNumPro(j), i * j);
            if (j != 0)
            {
                EXPECT_EQ(CppBigNumPro(i) / CppBigNumPro(j), i / j);
            }
        }
    }

    for (CppBigNumPro a = "189627391243132112356412332112563341892313212136512743681123231126653419862436125131629874398124362311233211238964";
         a < CppBigNumPro("189627391243132112356412332112563341892313212136512743681123231126653419862436125131629874398124362311233211238965");
         ++a)
    {
        auto b = a * a;
        EXPECT_EQ(b / a, a);
        EXPECT_EQ(b.Sqrt(), a);
    }

    EXPECT_EQ(CppBigNumPro("189627391243132112356412332112563341892313212136512743681123231126653419862436125131629874398124362311233211238964")
              * CppBigNumPro("56124135641123231165243654126354113221386127489763524513223416231345657456988746451673241352323148962436"),
              "10642673427401890287959143328411391642189930927246419475009998895455345122848209235404866983967924237303456059052419544157437954862459572950100014649960672276296454977189535827118789145330347503954873558823630655556304");
}
