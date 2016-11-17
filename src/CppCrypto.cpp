#include "CppCrypto.h"

#include <stdint.h>

#include <fstream>

#include <openssl/md5.h>

#include "CppString.h"

using namespace std;

string CppCrypto::Md5(const string &data)
{
    unsigned char md[MD5_DIGEST_LENGTH];

    MD5((unsigned char *)data.c_str(), data.size(), (unsigned char*)&md);
    return CppString::Hex2Str((const char *)md, MD5_DIGEST_LENGTH);
}

string CppCrypto::Md5File(const string &path)throw(CppException)
{
    unsigned char md[MD5_DIGEST_LENGTH];

    const uint32_t BUF_SIZE = 1024;
    char buf[BUF_SIZE];

    ifstream ifs(path.c_str(), ios::in);
    if (!ifs)
    {
        THROW("Open file[%s] fail", path.c_str());
    }

    MD5_CTX c;
    MD5_Init(&c);

    while (!ifs.eof())
    {
        ifs.read(buf, sizeof(buf));
        MD5_Update(&c, buf, ifs.gcount());
    }

    ifs.close();
    MD5_Final(md, &c);

    return CppString::Hex2Str((const char *)md, MD5_DIGEST_LENGTH);
}
