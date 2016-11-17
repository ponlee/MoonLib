#ifndef _CPP_FILE_H_
#define _CPP_FILE_H_

#include <stdint.h>

#include <string>

#include "CppLog.h"

using namespace std;

class CppFile
{
public:
    //************************************
    // Describe:  获取文件大小,如果无法打开文件,返回-1
    // Parameter: const string & filePath   本地文件路径
    // Returns:   std::int32_t              文件大小
    // Author:    moon
    //************************************
    static int32_t GetFileSize(const string &filePath);

    //************************************
    // Describe:  将字符串添加到指定文件末尾
    // Parameter: const string & filename   文件名
    // Parameter: const string & msg        要写入的内容
    // Returns:   void
    // Author:    moon
    //************************************
    static void AppendMsg(const string &filename, const string &msg);

    //************************************
    // Describe:  保存字符串内容到一个文件中,字符串中的数据可以是二进制数据,把它当做buf即可
    // Parameter: const string & filename   文件名
    // Parameter: const string & msg        要写入的内容
    // Returns:   void
    // Author:    moon
    //************************************
    static void WriteToFile(const string &filename, const string &msg);

    //************************************
    // Describe:  读取文件内容到字符串
    // Parameter: const string & filename
    // Returns:   void
    // Author:    moon
    //************************************
    static string ReadFromFile(const string &filename)throw(CppException);

    /** 复制文件
     *
     * @param   const string & srcFilePath
     * @param   const string & dstFilePath
     * @retval  void
     * @author  moontan
     */
    static void CopyFile(const string &srcFilePath, const string &dstFilePath)throw(CppException);

    /** 递归创建文件夹
     *
     * @param   const string & dir
     * @retval  int32_t             与mkdir的返回值相同
     * @author  moon
     */
    static int32_t MakeDir(const string &dir);

    /** 判断文件是否存在
     *
     * @param   const string & filePath
     * @retval  bool
     * @author  moontan
     */
    static bool IsFileExists(const string &filePath);
};

#endif
