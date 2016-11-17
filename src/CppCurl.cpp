#include "CppCurl.h"

#include <stdint.h>

#include <iostream>
#include <sstream>

#include "CppString.h"
#include "CppLog.h"
#include "CppFile.h"

using namespace std;

namespace CppCurlInner
{
//************************************
// Describe:  将libcurl读取的数据写入(string *)content
// Parameter: void * buffer     CURL读取到的数据
// Parameter: size_t size       读取的每个内存块字节数
// Parameter: size_t nmemb      读取的内存块数量
// Parameter: void * content    用户数据,保存数据的字符串
// Returns:   size_t            返回读取成功的字节数
// Author:    moon
//************************************
static size_t writeData(void *buffer, size_t size, size_t nmemb, void *outStream)
{
    if (buffer == NULL || outStream == NULL)
    {
        return 0;
    }

    int segsize = size * nmemb;

    ostream &fileStream = *(ostream *)outStream;
    fileStream.write((char *)buffer, segsize);

    return segsize;
}

//************************************
// Describe:  从文件流中读取数据传送给lincurl
// Parameter: void * ptr        数据目的地
// Parameter: size_t size       读取的每个内存块字节数
// Parameter: size_t nmemb      读取的内存块数量
// Parameter: void * stream     用户数据,(FILE *)stream,文件流
// Returns:   size_t
// Author:    moon
//************************************
// static size_t readData(void *ptr, size_t size, size_t nmemb, void *stream)
// {
//     if(stream == NULL || ptr == NULL)
//     {
//         return 0;
//     }
//
//     return fread(ptr, size, nmemb, (FILE *)stream);
// }
}

using namespace CppCurlInner;

static size_t readDataFromStream(void *ptr, size_t size, size_t nmemb, void *vFileStream)
{
    iostream &fileStream = *(iostream *)vFileStream;
    fileStream.read((char *)ptr, size * nmemb);

    return fileStream.gcount();
}

string CppCurl::Get(const string &url, const string &cookiesFile,
                    const vector<string> &otherHeaders, const string &proxy,
                    int32_t timeOut, CURL *curl)throw(CppException)
{
    stringstream result;
    char errorMsg[CURL_ERROR_SIZE];
    *errorMsg = '\0';

    try
    {
        if (curl == NULL)
        {
            curl = curl_easy_init();
        }

        if (!curl)
        {
            THROW("curl_easy_init错误");
        }

        // 添加其他需要的头
        curl_slist *http_headers = NULL;
        for (vector<string>::const_iterator it = otherHeaders.begin(); it != otherHeaders.end(); ++it)
        {
            http_headers = curl_slist_append(http_headers, it->c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);

        // 重定向跟随，设置最大限制为20次，避免循环阻塞
        // 比如如果不设置最大限制的话，http://www.iaea.org/inis/collection/NCLCollectionStore/_Public/44/096/44096722.pdf会有重定向循环，导致libcurl阻塞
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);

        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);        // 禁止重用,提高性能
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);           // 禁止DNS域名解析超时发送信号
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorMsg);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  // HTTPS不指定证书
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);  // HTTPS不指定证书

        if (cookiesFile.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
        }

        if (proxy.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        }

        if (timeOut > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
        }

        CURLcode res = curl_easy_perform(curl);
        if (res)
        {
            THROW("curl_easy_perform错误,url[%s],%s(%d)", url.c_str(), errorMsg, res);
        }
    }
    catch (...)
    {
        if (curl != NULL)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }
        throw;
    }

    if (curl != NULL)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }

    return result.str();
}

void CppCurl::Get(const string &url, const string &localPath,
                  const string &cookiesFile /*= ""*/, const vector<string> &otherHeaders /*= vector<string>()*/,
                  const string &proxy /*= ""*/, int32_t timeOut /*= 10*/, CURL *curl) throw(CppException)
{
    ofstream ofs(localPath.c_str(), ios::out);
    if (!ofs)
    {
        THROW("Open file[%s] fail", localPath.c_str());
    }

    char errorMsg[CURL_ERROR_SIZE];
    *errorMsg = '\0';

    try
    {
        if (curl == NULL)
        {
            curl = curl_easy_init();
        }

        if (!curl)
        {
            THROW("curl_easy_init错误");
        }

        // 添加其他需要的头
        curl_slist *http_headers = NULL;
        for (vector<string>::const_iterator it = otherHeaders.begin(); it != otherHeaders.end(); ++it)
        {
            http_headers = curl_slist_append(http_headers, it->c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&ofs);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);

        // 重定向跟随，设置最大限制为20次，避免循环阻塞
        // 比如如果不设置最大限制的话，http://www.iaea.org/inis/collection/NCLCollectionStore/_Public/44/096/44096722.pdf会有重定向循环，导致libcurl阻塞
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);

        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);        // 禁止重用,提高性能
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);           // 禁止DNS域名解析超时发送信号
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorMsg);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  // HTTPS不指定证书
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);  // HTTPS不指定证书

        if (cookiesFile.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
        }

        if (proxy.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        }

        if (timeOut > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
        }

        CURLcode res = curl_easy_perform(curl);
        if (res)
        {
            THROW("curl_easy_perform错误,url[%s],%s(%d)", url.c_str(), errorMsg, res);
        }
    }
    catch (...)
    {
        if (curl != NULL)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }
        throw;
    }

    if (curl != NULL)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }
}

string CppCurl::Post(const string &url, const string &data, const string &cookiesFile, const vector<string> &otherHeaders, const string &proxy, int32_t timeOut)throw(CppException)
{
    CURL *curl = NULL;
    stringstream result;
    char errorMsg[CURL_ERROR_SIZE];
    *errorMsg = '\0';

    try
    {
        curl = curl_easy_init();
        if (!curl)
        {
            THROW("curl_easy_init错误");
        }

        // 添加其他需要的头
        curl_slist *http_headers = NULL;
        for (vector<string>::const_iterator it = otherHeaders.begin(); it != otherHeaders.end(); ++it)
        {
            http_headers = curl_slist_append(http_headers, it->c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        // curl_easy_setopt(curl, CURLOPT_HEADER, 1);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);        // 禁止重用,提高性能
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);           // 禁止DNS域名解析超时发送信号
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorMsg);

        if (cookiesFile.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiesFile.c_str());
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
        }

        if (proxy.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        }

        if (timeOut > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
        }

        CURLcode res = curl_easy_perform(curl);
        if (res)
        {
            THROW("curl_easy_perform错误,url[%s],%s(%d)", url.c_str(), errorMsg, res);
        }
    }
    catch (...)
    {
        if (curl != NULL)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }
        throw;
    }

    if (curl != NULL)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }

    return result.str();
}

string CppCurl::PostForm(const string &url, const map<string, string> &formData, const string &cookiesFile /*= ""*/, const vector<string> &otherHeaders /*= vector<string>()*/, const string &proxy, int32_t timeOut)throw(CppException)
{
    CURL *curl = NULL;
    curl_httppost *post = NULL;
    stringstream result;
    char errorMsg[CURL_ERROR_SIZE];
    *errorMsg = '\0';

    try
    {
        curl = curl_easy_init();
        if (!curl)
        {
            THROW("curl_easy_init错误");
        }

        post = NULL;
        curl_httppost *last = NULL;

        // 添加表单
        for (map<string, string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
        {
            CURLFORMcode retCode;
            if (it->first == "Filedata")
            {
                retCode = curl_formadd(&post, &last,
                                       CURLFORM_COPYNAME, it->first.c_str(),
                                       CURLFORM_FILE, it->second.c_str(),
                                       CURLFORM_END);
            }
            else
            {
                retCode = curl_formadd(&post, &last,
                                       CURLFORM_COPYNAME, it->first.c_str(),
                                       CURLFORM_COPYCONTENTS, it->second.c_str(),
                                       CURLFORM_END);
            }

            if (retCode != CURL_FORMADD_OK)
            {
                THROW("curl_formadd错误.retCode=%d", retCode);
            }
        }

        // 添加其他需要的头
        curl_slist *http_headers = NULL;
        for (vector<string>::const_iterator it = otherHeaders.begin(); it != otherHeaders.end(); ++it)
        {
            http_headers = curl_slist_append(http_headers, (char *)it->c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
        }

        // 设置eash handle属性
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); /*Set URL*/
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);        // 禁止重用,提高性能
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);           // 禁止DNS域名解析超时发送信号
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorMsg);

        if (cookiesFile.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiesFile.c_str());
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
        }

        if (proxy.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        }

        if (timeOut > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
        }

        /* 读取应答 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);

        // 执行上传操作
        CURLcode res = curl_easy_perform(curl);
        if (res)
        {
            THROW("curl_easy_perform错误,url[%s],%s(%d)", url.c_str(), errorMsg, res);
        }
    }
    catch (...)
    {
        if (curl != NULL)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }

        if (post != NULL)
        {
            curl_formfree(post);
            post = NULL;
        }
        throw;
    }

    if (curl != NULL)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }

    if (post != NULL)
    {
        curl_formfree(post);
        post = NULL;
    }

    return result.str();
}

string CppCurl::PostFileData(const string &url, const map<string, string> &formData, const FormFile &formFile,
                             const string &fileData, const string &cookiesFile /*= ""*/,
                             const vector<string> &otherHeaders /*= vector<string>()*/,
                             const string &proxy, int32_t timeOut)throw(CppException)
{
    CURL *curl = NULL;
    curl_httppost *post = NULL;
    stringstream result;
    stringstream fileStream(fileData);
    char errorMsg[CURL_ERROR_SIZE];
    *errorMsg = '\0';

    try
    {
        curl = curl_easy_init();
        if (!curl)
        {
            THROW("curl_easy_init错误");
        }

        post = NULL;
        curl_httppost *last = NULL;

        // 添加表单
        CURLFORMcode retCode;
        for (map<string, string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
        {
            retCode = curl_formadd(&post, &last,
                                   CURLFORM_COPYNAME, it->first.c_str(),
                                   CURLFORM_COPYCONTENTS, it->second.c_str(),
                                   CURLFORM_END);
            CHECK_THROW_F(retCode == CURL_FORMADD_OK, "curl_formadd错误.retCode=%d", retCode);
        }

        // 添加文件表单
        retCode = curl_formadd(&post, &last,
                               CURLFORM_COPYNAME, formFile.FormName.c_str(),
                               CURLFORM_FILENAME, formFile.FileName.c_str(),
                               CURLFORM_CONTENTTYPE, formFile.ContentType.c_str(),
                               CURLFORM_CONTENTSLENGTH, fileData.length(),
                               CURLFORM_STREAM, &fileStream,
                               CURLFORM_END);
        CHECK_THROW_F(retCode == CURL_FORMADD_OK, "curl_formadd错误.retCode=%d", retCode);

        // 添加其他需要的头
        curl_slist *http_headers = NULL;
        for (vector<string>::const_iterator it = otherHeaders.begin(); it != otherHeaders.end(); ++it)
        {
            http_headers = curl_slist_append(http_headers, (char *)it->c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
        }

        // 设置eash handle属性
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); /*Set URL*/
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readDataFromStream);
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);        // 禁止重用,提高性能
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);           // 禁止DNS域名解析超时发送信号
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorMsg);

        if (cookiesFile.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiesFile.c_str());
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
        }

        if (proxy.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        }

        if (timeOut > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
        }

        /* 读取应答 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);

        // 执行上传操作
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            THROW("curl_easy_perform错误,url[%s],%s(%d)", url.c_str(), errorMsg, res);
        }
    }
    catch (...)
    {
        if (curl != NULL)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }

        if (post != NULL)
        {
            curl_formfree(post);
            post = NULL;
        }
        throw;
    }

    if (curl != NULL)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }

    if (post != NULL)
    {
        curl_formfree(post);
        post = NULL;
    }

    return result.str();
}

string CppCurl::PostFile(const string &url, const map<string, string> &formData, const FormFile &formFile,
                         const string &localPath, const string &cookiesFile /*= ""*/,
                         const vector<string> &otherHeaders /*= vector<string>()*/,
                         const string &proxy, int32_t timeOut)throw(CppException)
{
    CURL *curl = NULL;
    curl_httppost *post = NULL;
    stringstream result;
    ifstream fileStream(localPath.c_str(), ios::in);
    if (!fileStream)
    {
        THROW("Open file[%s] fail", localPath.c_str());
    }

    uint32_t fileSize = CppFile::GetFileSize(localPath);
    char errorMsg[CURL_ERROR_SIZE];
    *errorMsg = '\0';

    try
    {
        curl = curl_easy_init();
        if (!curl)
        {
            THROW("curl_easy_init错误");
        }

        post = NULL;
        curl_httppost *last = NULL;

        // 添加表单
        CURLFORMcode retCode;
        for (map<string, string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
        {
            retCode = curl_formadd(&post, &last,
                                   CURLFORM_COPYNAME, it->first.c_str(),
                                   CURLFORM_COPYCONTENTS, it->second.c_str(),
                                   CURLFORM_END);
            CHECK_THROW_F(retCode == CURL_FORMADD_OK, "curl_formadd错误.retCode=%d", retCode);
        }

        // 添加文件表单
        retCode = curl_formadd(&post, &last,
                               CURLFORM_COPYNAME, formFile.FormName.c_str(),
                               CURLFORM_FILENAME, formFile.FileName.c_str(),
                               CURLFORM_CONTENTTYPE, formFile.ContentType.c_str(),
                               CURLFORM_CONTENTSLENGTH, fileSize,
                               CURLFORM_STREAM, &fileStream,
                               CURLFORM_END);
        CHECK_THROW_F(retCode == CURL_FORMADD_OK, "curl_formadd错误.retCode=%d", retCode);

        // 添加其他需要的头
        curl_slist *http_headers = NULL;
        for (vector<string>::const_iterator it = otherHeaders.begin(); it != otherHeaders.end(); ++it)
        {
            http_headers = curl_slist_append(http_headers, (char *)it->c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers);
        }

        // 设置eash handle属性
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); /*Set URL*/
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readDataFromStream);
        curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);        // 禁止重用,提高性能
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);           // 禁止DNS域名解析超时发送信号
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorMsg);

        if (cookiesFile.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiesFile.c_str());
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiesFile.c_str());
        }

        if (proxy.length() != 0)
        {
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
        }

        if (timeOut > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
        }

        /* 读取应答 */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&result);

        // 执行上传操作
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            THROW("curl_easy_perform错误,url[%s],%s(%d)", url.c_str(), errorMsg, res);
        }
    }
    catch (...)
    {
        if (curl != NULL)
        {
            curl_easy_cleanup(curl);
            curl = NULL;
        }

        if (post != NULL)
        {
            curl_formfree(post);
            post = NULL;
        }
        throw;
    }

    if (curl != NULL)
    {
        curl_easy_cleanup(curl);
        curl = NULL;
    }

    if (post != NULL)
    {
        curl_formfree(post);
        post = NULL;
    }

    return result.str();
}

void CppCurl::Init(long flags)throw(CppException)
{
    CURLcode ret = curl_global_init(flags);

    if (ret != CURLE_OK)
    {
        THROW("curl_global_init错误,ret=%d", ret);
    }
}

string CppCurl::GetHttpParam(map<string, string> &params)
{
    string result;
    for (auto param_it = params.begin(); param_it != params.end(); ++param_it)
    {
        result += param_it->first + "=" + param_it->second + "&";
    }

    // 去除最后一个&符号
    if (!result.empty())
    {
        result.erase(result.end() - 1);
    }

    return result;
}
