#ifndef __CYGWIN__
#include <assert.h>

#include <iostream>

#include <mysqlpp/mysql++.h>

#include "gtest/gtest.h"

using namespace std;

TEST(CppMysqlpp, Test)
{
//     mysqlpp::Connection conn(false);
//     bool connectResult = conn.connect("docup", "127.0.0.1", "root", "docuproot");
//     EXPECT_TRUE(connectResult);
// 
//     mysqlpp::Query query = conn.query("SELECT * FROM config;");
//     mysqlpp::StoreQueryResult res = query.store();
//     EXPECT_TRUE(res);
//     uint32_t resSize = res.size();
//     EXPECT_GT(resSize, 0);

    // 可以输出结果看看
//     mysqlpp::StoreQueryResult::const_iterator it;
//     for (it = res.begin(); it != res.end(); ++it)
//     {
//         mysqlpp::Row row = *it;
//         cout << '\t' << row[0] << ":" << row[1] << endl;
//     }
}

#endif
