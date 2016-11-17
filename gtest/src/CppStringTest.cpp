#include <iostream>

#include "gtest/gtest.h"

#include <CppString.h>

using namespace std;

TEST(CppString, ReverseTest)
{
    EXPECT_EQ(CppString::Reverse("asdasd"), "dsadsa");
    EXPECT_EQ(CppString::Reverse(""), "");
    EXPECT_EQ(CppString::Reverse("12"), "21");
    EXPECT_EQ(CppString::Reverse("1"), "1");
}

TEST(CppString, ToStringTest)
{
    EXPECT_EQ("123", CppString::ToString(123));
}

TEST(CppString, CppStringFromString)
{
    string strValue("123");
    uint32_t intValue = CppString::FromString<uint32_t>(strValue);
    EXPECT_EQ(123u, intValue);
}

TEST(CppString, RemoveAngleTest)
{
    EXPECT_EQ("123", CppString::RemoveAngle("123[123]", '[', ']'));
    EXPECT_EQ("123[123", CppString::RemoveAngle("123[123", '[', ']'));
    EXPECT_EQ("123", CppString::RemoveAngle("123(123)", '(', ')'));
    EXPECT_EQ("", CppString::RemoveAngle("(123123)", '(', ')'));
    EXPECT_EQ("", CppString::RemoveAngle("(123(123)", '(', ')'));
    EXPECT_EQ("", CppString::RemoveAngle("(123(123]", '(', ']'));
    EXPECT_EQ("asddsa", CppString::RemoveAngle("asd(123(123]dsa", '(', ']'));
    EXPECT_EQ("3213210", CppString::RemoveAngle("321'123123'3210", '\'', '\''));
}

TEST(CppString, TrimTest)
{
    vector<string> trimStrs;
    trimStrs.push_back("wo");
    trimStrs.push_back("shi");

    EXPECT_EQ("", CppString::TrimLeft(""));
    EXPECT_EQ("", CppString::TrimRight(""));
    EXPECT_EQ("", CppString::TrimLeft("   ", -1));
    EXPECT_EQ("", CppString::TrimRight("   ", -1));
    EXPECT_EQ("shizhu", CppString::TrimLeft("woshizhu", "wo"));
    EXPECT_EQ("woshizhu", CppString::TrimLeft("woshizhu", ""));
    EXPECT_EQ("", CppString::TrimLeft("woshizhu", "woshizhu"));
    EXPECT_EQ("shiwozhuwo", CppString::TrimLeft("woshiwozhuwo", "wo"));
    EXPECT_EQ("woshizhu", CppString::TrimLeft("wowowoshizhu", "wo", 2));
    EXPECT_EQ("shizhu", CppString::TrimLeft("wowowoshizhu", "wo", 3));
    EXPECT_EQ("shizhu", CppString::TrimLeft("wowowowoshizhu", "wo", -1));
    EXPECT_EQ("wowowowoshizhu", CppString::TrimLeft("wowowowoshizhu", "wo", 0));

    EXPECT_EQ("shiwozhuwo", CppString::TrimLeft("woshiwozhuwo", trimStrs));
    EXPECT_EQ("zhuwo", CppString::TrimLeft("woshiwozhuwo", trimStrs, -1));
    EXPECT_EQ("wozhuwo", CppString::TrimLeft("woshiwozhuwo", trimStrs, 2));

    EXPECT_EQ("wowowowoshizhu", CppString::TrimLeft("  \t\r\n\b  wowowowoshizhu", -1));
    EXPECT_EQ("  wowowowoshizhu", CppString::TrimLeft("  wowowowoshizhu", 0));
    EXPECT_EQ(" wowowowoshizhu", CppString::TrimLeft("  wowowowoshizhu", 1));
    EXPECT_EQ("wowowowoshizhu", CppString::TrimLeft("  wowowowoshizhu", 2));
    EXPECT_EQ("wowowowoshizhu", CppString::TrimLeft("  wowowowoshizhu", 3));


    EXPECT_EQ("woshi", CppString::TrimRight("woshizhu", "zhu"));
    EXPECT_EQ("woshizhu", CppString::TrimRight("woshizhu", ""));
    EXPECT_EQ("", CppString::TrimRight("woshizhu", "woshizhu"));
    EXPECT_EQ("woshiwozhu", CppString::TrimRight("woshiwozhu", "wo"));
    EXPECT_EQ("wowowoshizhuzhu", CppString::TrimRight("wowowoshizhuzhuzhuzhu", "zhu", 2));
    EXPECT_EQ("wowowoshizhu", CppString::TrimRight("wowowoshizhuzhuzhuzhu", "zhu", 3));
    EXPECT_EQ("wowowoshi", CppString::TrimRight("wowowoshizhuzhuzhuzhu", "zhu", -1));
    EXPECT_EQ("wowowoshizhuzhuzhuzhu", CppString::TrimRight("wowowoshizhuzhuzhuzhu", "zhu", 0));

    EXPECT_EQ("woshiwozhuwoshi", CppString::TrimRight("woshiwozhuwoshiwo", trimStrs));
    EXPECT_EQ("woshiwozhu", CppString::TrimRight("woshiwozhuwoshiwo", trimStrs, -1));
    EXPECT_EQ("woshiwozhuwo", CppString::TrimRight("woshiwozhuwoshiwo", trimStrs, 2));

    EXPECT_EQ("wowowowoshizhu", CppString::TrimRight("wowowowoshizhu  \t\r\n\b  ", -1));
    EXPECT_EQ("wowowowoshizhu  ", CppString::TrimRight("wowowowoshizhu  ", 0));
    EXPECT_EQ("wowowowoshizhu ", CppString::TrimRight("wowowowoshizhu  ", 1));
    EXPECT_EQ("wowowowoshizhu", CppString::TrimRight("wowowowoshizhu  ", 2));
    EXPECT_EQ("wowowowoshizhu", CppString::TrimRight("wowowowoshizhu  ", 3));


    EXPECT_EQ("shizhu", CppString::Trim("woshizhuwo", "wo"));
    EXPECT_EQ("wowo", CppString::Trim("wowowowo", "wo"));
    EXPECT_EQ("wowowowo", CppString::Trim("wowowowo", ""));
    EXPECT_EQ("", CppString::Trim("wowowowo", "wowowowo"));
    EXPECT_EQ("woshizhuwowo", CppString::Trim("wowowoshizhuwowowowo", "wo", 2));
    EXPECT_EQ("shizhuwowo", CppString::Trim("wowowoshizhuwowowowowo", "wo", 3));
    EXPECT_EQ("shiwozhu", CppString::Trim("wowowoshiwozhuwowowowowowowowowo", "wo", -1));
    EXPECT_EQ("wowowoshiwozhuwowowowowowowowowo", CppString::Trim("wowowoshiwozhuwowowowowowowowowo", "wo", 0));

    EXPECT_EQ("shiwozhuwoshi", CppString::Trim("woshiwozhuwoshiwo", trimStrs));
    EXPECT_EQ("zhu", CppString::Trim("woshiwozhuwoshiwo", trimStrs, -1));
    EXPECT_EQ("wozhuwo", CppString::Trim("woshiwozhuwoshiwo", trimStrs, 2));

    EXPECT_EQ("wowowowoshizhu", CppString::Trim("  \t\r\n\b  wowowowoshizhu  \t\r\n\b  ", -1));
    EXPECT_EQ("  wowowowoshizhu  ", CppString::Trim("  wowowowoshizhu  ", 0));
    EXPECT_EQ(" wowowowoshizhu ", CppString::Trim("  wowowowoshizhu  ", 1));
    EXPECT_EQ("wowowowoshizhu", CppString::Trim("wowowowoshizhu  ", 2));
    EXPECT_EQ("wowowowoshizhu", CppString::Trim("  wowowowoshizhu  ", 3));
}

TEST(CppString, SplitStr)
{
    vector<string> result;
    CppString::SplitStr("123", "", result);
    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("123", result[0]);

    CppString::SplitStr("123", "2", result);
    EXPECT_EQ(2u, result.size());
    EXPECT_EQ("1", result[0]);
    EXPECT_EQ("3", result[1]);

    CppString::SplitStr(";;;;123;;;;234;;;;345;;;;", ";", result);
    EXPECT_EQ(3u, result.size());
    EXPECT_EQ("123", result[0]);
    EXPECT_EQ("234", result[1]);
    EXPECT_EQ("345", result[2]);

    CppString::SplitStr("", ";", result);
    EXPECT_EQ(0u, result.size());

    CppString::SplitStr("aaa;bbb;ccc;", ";", result, false);
    EXPECT_EQ(4u, result.size());
    EXPECT_EQ("", result[3]);

    CppString::SplitStr("aaa;bbb;ccc;;", ";", result, false);
    EXPECT_EQ(5u, result.size());
    EXPECT_EQ("", result[3]);
    EXPECT_EQ("", result[4]);

    CppString::SplitStr("aaa;bbb;ccc;;", ";", result, true, 2);
    EXPECT_EQ(2u, result.size());
    EXPECT_EQ("aaa", result[0]);
    EXPECT_EQ("bbb;ccc;;", result[1]);

    CppString::SplitStr("", ";", result, true, 2);
    EXPECT_EQ(0u, result.size());

    CppString::SplitStr("aaa;bbb;ccc;;", ";", result, true, 1);
    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("aaa;bbb;ccc;;", result[0]);

    CppString::SplitStr("test a   #55", "#", result, true, 2);
    EXPECT_EQ(2u, result.size());
    EXPECT_EQ("test a   ", result[0]);
    EXPECT_EQ("55", result[1]);

    CppString::SplitStr("test a   ", "#", result, true, 2);
    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("test a   ", result[0]);

    CppString::SplitStr("test a   ", " ", result, true, 2);
    EXPECT_EQ(2u, result.size());
    EXPECT_EQ("test", result[0]);
    EXPECT_EQ("a   ", result[1]);

    CppString::SplitStr("test a   ", " ", result, true, 3);
    EXPECT_EQ(2u, result.size());
    EXPECT_EQ("test", result[0]);
    EXPECT_EQ("a", result[1]);

    CppString::SplitStr("test    ", " ", result, true, 2);
    EXPECT_EQ(1u, result.size());
    EXPECT_EQ("test", result[0]);
}

TEST(CppString, GetArgsTest)
{
    EXPECT_EQ("This is a test", CppString::GetArgs("This is a %s", "test"));

    int intValue = 20;
    EXPECT_EQ("intValue = 20", CppString::GetArgs("intValue = %d", intValue));
}

TEST(CppString, Hex2Str)
{
    char buf[] = "123456789";
    EXPECT_EQ("313233343536373839", CppString::Hex2Str(buf));
    EXPECT_EQ("313233343536373839", CppString::Hex2Str(buf, true));

    buf[0] = 0x5A;
    EXPECT_EQ("5a3233343536373839", CppString::Hex2Str(buf));
    EXPECT_EQ("5A3233343536373839", CppString::Hex2Str(buf, true));
}
