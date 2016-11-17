#include "CppMysql.h"

void CppMysql::StoreResultToMap(CMysql &mysqlConn, map<string, vector<string> > &data)
{
    data.clear();

    MYSQL_ROW row;
    bool isFirstRow = true;
    mysqlConn.StoreResult();
    while ((row = mysqlConn.FetchRow()) != NULL)
    {
        for (uint32_t colIndex = 0; colIndex < mysqlConn.GetAffectedCols(); ++colIndex)
        {
            if (isFirstRow)
            {
                data[mysqlConn.GetIndexFieldName(colIndex)];
            }

            data[mysqlConn.GetIndexFieldName(colIndex)].push_back(row[colIndex] == NULL ? "" : row[colIndex]);
        }

        isFirstRow = false;
    }
}

string CppMysql::GetMysqlResult(const map<string, vector<string> > &data, const string &feildName, uint32_t row)
{
    map<string, vector<string> >::const_iterator it = data.find(feildName);
    if (it == data.end())
    {
        return "";
    }

    if (row >= it->second.size())
    {
        return "";
    }

    return it->second[row];
}
