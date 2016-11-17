#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include <CppLog.h>
#include "global.h"

using namespace std;

#define MAKE_CLASS(class_name,...) class_name(){__VA_ARGS__;}

class  A
{
public:
    MAKE_CLASS(A);
};

class  B
{
public:
    MAKE_CLASS(B, a = false);

    bool a;
};

class  C
{
public:
    MAKE_CLASS(C, a = true);

    bool a;
};

TEST(CppLog, MacroTest)
{
    C c;
    B b;
    EXPECT_TRUE(c.a);
    EXPECT_FALSE(b.a);
}

// TEST(CppLog, LogTest)
// {
//     const string FILE_PATH = "/tmp/CppLogTest.txt";
//     CppLog cppLog(FILE_PATH, CppLog::DEBUG, 65, 3);
// 
//     for(uint32_t i = 0; i <= 1000; ++i)
//     {
//         DEBUG_LOG("a");
//     }
// }
