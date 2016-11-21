#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include <CppCurl.h>
#include <CppRegex.h>

using namespace std;

TEST(CppTest, DISABLED_DTSMusic)
{
    // 列表地址：http://blog.sina.com.cn/s/articlelist_1197845451_8_1.html

    CppRegex titleReg("<title>.*?</title>");
    CppRegex listReg("http://blog.sina.com.cn/s/blog_.*?html");
    for (uint32_t page = 1; page <= 20; ++page)
    {
        string html = CppCurl::Get(CppString::GetArgs("http://blog.sina.com.cn/s/articlelist_1197845451_8_%d.html", page));
        vector<string> result;
        listReg.Matches(html, result);
        for (auto result_it = result.begin(); result_it != result.end(); ++result_it)
        {
            html = CppCurl::Get(*result_it);
            string title = CppString::ReplaceStr(CppString::ReplaceStr(titleReg.Match(html), "_jiangkuaihe_新浪博客</title>"), "<title>");
            title = CppString::ReplaceStr(title, "&nbsp;", " ");
            cout << title << endl << *result_it << endl << endl;
        }
    }
}

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
