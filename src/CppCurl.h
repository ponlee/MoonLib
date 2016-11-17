#ifndef _CPP_CURL_H_
#define _CPP_CURL_H_

#include <string>
#include <vector>
#include <map>

#include <curl/curl.h>

#include "CppLog.h"

using std::string;

struct FormFile
{
    string FormName;            // 表格名，一般是file或者FileData
    string FileName;            // 文件名，按照正常的填写即可
    string ContentType;         // 流类型，默认application/octet-stream

    FormFile() :ContentType("application/octet-stream")
    {
    }
};

class CppCurl
{
public:
    //************************************
    // Describe:  Get方式获取数据
    // Parameter: const string & url
    // Parameter: const string & cookiesFile
    // Returns:   string
    // Author:    moon
    //************************************
    static string Get(const string &url, const string &cookiesFile = "",
                      const vector<string> &otherHeaders = vector<string>(),
                      const string &proxy = "", int32_t timeOut = 10, CURL *curl = NULL) throw(CppException);

    //************************************
    // Describe:  Get方式获取数据到本地文件
    // Parameter: const string & url
    // Parameter: const string & cookiesFile
    // Returns:   string
    // Author:    moon
    //************************************
    static void Get(const string &url, const string &localPath, const string &cookiesFile = "",
                    const vector<string> &otherHeaders = vector<string>(),
                    const string &proxy = "", int32_t timeOut = 10, CURL *curl = NULL) throw(CppException);

    //************************************
    // Describe:  Post方式传输数据
    // Parameter: const string & url
    // Parameter: const string & data
    // Parameter: const string & cookiesFile
    // Parameter: const vector<string> & otherHeaders
    // Returns:   string
    // Author:    moon
    //************************************
    static string Post(const string &url, const string &data, const string &cookiesFile = "",
                       const vector<string> &otherHeaders = vector<string>(),
                       const string &proxy = "", int32_t timeOut = 10)throw(CppException);

    //************************************
    // Describe:  Post表格数据
    // Parameter: const string & url
    // Parameter: const map<string, string> & formData
    // Parameter: const string & cookiesFile
    // Parameter: const vector<string> & otherHeaders
    // Returns:   string
    // Author:    moon
    //************************************
    static string PostForm(const string &url, const map<string, string> &formData,
                           const string &cookiesFile = "",
                           const vector<string> &otherHeaders = vector<string>(),
                           const string &proxy = "", int32_t timeOut = 10)throw(CppException);

    /** 发送文件,文件内存保存在string类型中
     *
     * @param   const string & url
     * @param   const map<string, string> & formData
     * @param   const FormFile & formFile
     * @param   const string & fileData
     * @param   const string & cookiesFile
     * @param   const vector<string> & otherHeaders
     * @param   const string & proxy
     * @param   int32_t timeOut
     * @retval  string
     * @author  moontan
     */
    static string PostFileData(const string &url, const map<string, string> &formData,
                               const FormFile &formFile, const string &fileData,
                               const string &cookiesFile = "",
                               const vector<string> &otherHeaders = vector<string>(),
                               const string &proxy = "", int32_t timeOut = 10)throw(CppException);

    /** 上传本地文件
     *
     * @param   const string & url
     * @param   const map<string, string> & formData
     * @param   const FormFile & formFile
     * @param   const string & localPath
     * @param   const string & cookiesFile
     * @param   const vector<string> & otherHeaders
     * @param   const string & proxy
     * @param   int32_t timeOut
     * @retval  string
     * @author  moontan
     */
    static string PostFile(const string &url, const map<string, string> &formData,
                           const FormFile &formFile, const string &localPath,
                           const string &cookiesFile = "",
                           const vector<string> &otherHeaders = vector<string>(),
                           const string &proxy = "", int32_t timeOut = 10)throw(CppException);

    static void Init(long flags = CURL_GLOBAL_ALL)throw(CppException);
    //************************************
    // Describe:  将字符串编码成URL
    // Parameter: const string & data
    // Returns:   string
    // Author:    moon
    //************************************
    /*    static string GetUrlEncode(const string &data);*/

    /** 获得请求内容
     *  TODO 暂时不考虑特殊符号，比如param内有'='或者'&'，这部分可以转换为%**的格式
     *
     * @param   map<string, string> & params    比如params["a"]="1",params["b"]="2"
     * @retval  string                          返回a=1&b=2
     * @author  moontan
     */
    static string GetHttpParam(map<string, string> &params);
};

#endif
