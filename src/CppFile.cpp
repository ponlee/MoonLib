#include "CppFile.h"

#include <stdlib.h>

#include <fstream>

#include "CppString.h"

using namespace std;

bool CppFile::IsFileExists(const string &filePath)
{
    if (ifstream(filePath.c_str()))
    {
        return true;
    }

    return false;
}

int32_t CppFile::GetFileSize(const string &filePath)
{
    ifstream ifs(filePath.c_str());

    // 文件不存在或者打开失败
    if (!ifs)
    {
        return -1;
    }

    ifs.seekg(0, ios::end);

    int size = ifs.tellg();
    ifs.close();
    return size;
}

void CppFile::AppendMsg(const string &filename, const string &msg)
{
    ofstream file(filename.c_str(), ios::out | ios::app);
    if (!file)
    {
        return;
    }

    file << msg;
    file.close();
}

void CppFile::WriteToFile(const string &filename, const string &msg)
{
    ofstream file(filename.c_str(), ios::out);
    if (!file)
    {
        return;
    }

    file << msg;
    file.close();
}

string CppFile::ReadFromFile(const string &filename)throw(CppException)
{
    ifstream file(filename.c_str(), ios::in);
    if (!file)
    {
        THROW("Can't open file[%s]", filename.c_str());
    }

    std::string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    return content;
}

void CppFile::CopyFile(const string &srcFilePath, const string &dstFilePath) throw(CppException)
{
    ifstream srcFile(srcFilePath.c_str(), ios::in);
    if (!srcFile)
    {
        THROW("Can't open file[%s]", srcFilePath.c_str());
    }

    ofstream dstFile(dstFilePath.c_str(), ios::out);
    if (!dstFile)
    {
        THROW("Can't open file[%s]", srcFilePath.c_str());
    }

    char buf[2048];
    while (!srcFile.eof())
    {
        srcFile.read(buf, sizeof(buf));
        dstFile.write(buf, srcFile.gcount());
    }
}

int32_t CppFile::MakeDir(const string &dir)
{
    return system(CppString::GetArgs("mkdir -p %s", dir.c_str()).c_str());
}
