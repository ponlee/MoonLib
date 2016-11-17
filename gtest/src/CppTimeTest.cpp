#include <iostream>

#include "gtest/gtest.h"

#include <CppTime.h>

using namespace std;

TEST(CppTime, GetTimeFromStr)
{
    EXPECT_EQ(1373212862, CppTime::GetTimeFromStr("2013-07-08 00:01:02"));
    EXPECT_EQ(1373212860, CppTime::GetTimeFromStr("2013-07-08 00:01"));
    EXPECT_EQ(1373212860, CppTime::GetTimeFromStr("2013/07/08 00:01"));
    EXPECT_EQ(1373212860, CppTime::GetTimeFromStr("2013/7/8 00:01"));
    EXPECT_EQ(1373212800, CppTime::GetTimeFromStr("2013/7/8 00"));
    EXPECT_EQ(1373212800, CppTime::GetTimeFromStr("2013/7/8"));
    EXPECT_EQ(1372521600, CppTime::GetTimeFromStr("2013/7"));
    EXPECT_EQ(1372521600, CppTime::GetTimeFromStr("2013/7/"));
}

TEST(CppTime, IsSameDayTest)
{
    EXPECT_TRUE(CppTime::IsSameDay(time(NULL)));
    EXPECT_TRUE(CppTime::IsSameDay(CppTime::GetTimeFromStr("2013-07-08"), CppTime::GetTimeFromStr("2013-07-08")));
    EXPECT_FALSE(CppTime::IsSameDay(CppTime::GetTimeFromStr("2013-07-08"), CppTime::GetTimeFromStr("2013-07-09")));
    EXPECT_FALSE(CppTime::IsSameDay(CppTime::GetTimeFromStr("2013-07-08 00:01:02"), CppTime::GetTimeFromStr("2013-07-09 23:59:59")));
    EXPECT_TRUE(CppTime::IsSameDay(CppTime::GetTimeFromStr("2013-07-09 00:00:00"), CppTime::GetTimeFromStr("2013-07-09 23:59:59")));
}
