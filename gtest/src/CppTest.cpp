#include <iostream>
#include <string>

#include "gtest/gtest.h"

using namespace std;

TEST(CppTest, VTable)
{
    class Base
    {
    public:
        virtual string f()
        {
            return "Base::f";
        }
        virtual string g()
        {
            return "Base::g";
        }
        virtual string h()
        {
            return "Base::h";
        }
    };

    typedef string(*Fun)(void);
    Base b;
    Fun pFun = NULL;

    //     cout << "虚函数表地址：" << (int64_t*)(&b) << endl;
    //     cout << "虚函数表 ― 第1个函数地址：" << (int64_t*)*(int64_t*)(&b) << endl;
    //     cout << "虚函数表 ― 第2个函数地址：" << (int64_t*)*(int64_t*)(&b) + 1 << endl;
    //     cout << "虚函数表 ― 第3个函数地址：" << (int64_t*)*(int64_t*)(&b) + 2 << endl;

    // Invoke the first virtual function
    pFun = reinterpret_cast<Fun>(*(reinterpret_cast<int64_t*>(*reinterpret_cast<int64_t*>(&b))));
    EXPECT_EQ("Base::f", pFun());
    pFun = reinterpret_cast<Fun>(*(reinterpret_cast<int64_t*>(*reinterpret_cast<int64_t*>(&b)) + 1));
    EXPECT_EQ("Base::g", pFun());
    pFun = reinterpret_cast<Fun>(*(reinterpret_cast<int64_t*>(*reinterpret_cast<int64_t*>(&b)) + 2));
    EXPECT_EQ("Base::h", pFun());
}
