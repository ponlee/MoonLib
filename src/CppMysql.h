#ifndef _CPP_MYSQL_H_
#define _CPP_MYSQL_H_

#include <string>
#include <map>
#include <vector>

#include <cmysql/cmysql.h>

#include "CppString.h"
#ifndef USE_CPP_LOG_MACRO
#define USE_CPP_LOG_MACRO
#endif
#include "CppLog.h"

class CppMysql
{
public:
    static void MysqlQuery(MYSQL &mysql, const std::string &sqlCmd, CppLog *pCppLog = NULL)
    {
        Log(pCppLog, CppLog::DEBUG, sqlCmd.c_str());

        if (mysql_query(&mysql, sqlCmd.c_str()))
        {
            THROW("mysql_query[%s]错误:%s", sqlCmd.c_str(), mysql_error(&mysql));
        }
    }

    static void MysqlQuery(CMysql &mysql, const std::string &sqlCmd, CppLog *pCppLog = NULL) throw(CMysqlException)
    {
        Log(pCppLog, CppLog::DEBUG, sqlCmd.c_str());

        mysql.FreeResult();
        mysql.Query(sqlCmd.c_str());
    }

    static void StoreResultToMap(CMysql &mysql, std::map<std::string, std::vector<std::string> > &data);

    static std::string GetMysqlResult(const std::map<std::string, std::vector<std::string> > &data,
                                      const std::string &feildName, uint32_t row);
};
#endif
