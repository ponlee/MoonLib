#include <iostream>

#include <stdint.h>
#include "gtest/gtest.h"

#include <string>

using namespace std;

// snprintf
// 正确用法：snprintf(buf5, sizeof(buf5), "123456");
TEST(string, snprintf)
{
    char buf5[5];

    snprintf(buf5, sizeof(buf5), "123456");
    EXPECT_EQ("1234", string(buf5));

    snprintf(buf5, sizeof(buf5), "12345");
    EXPECT_EQ("1234", string(buf5));

    snprintf(buf5, sizeof(buf5), "1234");
    EXPECT_EQ("1234", string(buf5));

    snprintf(buf5, sizeof(buf5), "123");
    EXPECT_EQ("123", string(buf5));

    snprintf(buf5, sizeof(buf5) - 1, "123456");
    EXPECT_EQ("123", string(buf5));

    snprintf(buf5, sizeof(buf5) - 1, "12345");
    EXPECT_EQ("123", string(buf5));

    snprintf(buf5, sizeof(buf5) - 1, "1234");
    EXPECT_EQ("123", string(buf5));

    snprintf(buf5, sizeof(buf5) - 1, "123");
    EXPECT_EQ("123", string(buf5));

    snprintf(buf5, sizeof(buf5) - 1, "12");
    EXPECT_EQ("12", string(buf5));

    // 不会将后面的值填0
    buf5[sizeof(buf5) - 1] = 0xaa;
    snprintf(buf5, sizeof(buf5), "1");
    EXPECT_EQ("1", string(buf5));
    EXPECT_EQ((char)0xaa, buf5[sizeof(buf5) - 1]);
}

// strncat
// 正确用法：strncat(buf5, "123456", sizeof(buf5) - strlen(buf5) - 1);
TEST(string, strncat)
{
    char buf5[5];

    // 正确用法
    *buf5 = '\0';
    strncat(buf5, "123456", sizeof(buf5) - strlen(buf5) - 1);
    EXPECT_EQ("1234", string(buf5));

    strncat(buf5, "123456", sizeof(buf5) - strlen(buf5) - 1);
    EXPECT_EQ("1234", string(buf5));

    *buf5 = '\0';
    strncat(buf5, "123", sizeof(buf5) - strlen(buf5) - 1);
    EXPECT_EQ("123", string(buf5));

    strncat(buf5, "123", sizeof(buf5) - strlen(buf5) - 1);
    EXPECT_EQ("1231", string(buf5));
}

// strncat
// 正确用法：
// strncpy(str1,str2,sizeof(str1)-1);
// str1[sizeof(str1) - 1] = '/ 0';
TEST(string, strncpy)
{
    char a = 0;
    char buf5[5];
    char b = 0;     // ab用于给buf5的前后填充0
    (void)a;
    (void)b;

    // 当原串长度超过num时，strncpy不会在后面补0，一个坑
    buf5[3] = 'a';
    buf5[4] = '\0';
    strncpy(buf5, "123456", 3);
    EXPECT_EQ("123a", string(buf5));

    // 当原串长度不足时，会往后面的字节全补0直到num个字符
    char bufZero[10] = { 0 };
    buf5[sizeof(buf5) - 1] = 0xaa;
    strncpy(buf5, "1", sizeof(buf5));
    EXPECT_EQ("1", string(buf5));
    EXPECT_EQ(0, memcmp(buf5 + 1, bufZero, 4));
}
// 
// #include <CppString.h>
// 
// int32_t GetEscape(const std::string &template_string,
//                   std::string::size_type pos,
//                   std::string &out_string)
// {
//     if (template_string[pos] == '%')
//     {
//         out_string.append(1, '%');
//         return 1;
//     }
// 
//     if (isdigit(template_string[pos]))
//     {
//         uint32_t digit_count = template_string[pos] - '0';
//         if (template_string.size() <= (pos + digit_count))
//         {
//             return -1;
//         }
// 
//         int32_t cmd = CppString::FromString<int32_t>(string(template_string.data() + pos, digit_count + 1));
// 
//         switch (cmd)
//         {
//         case 3001:
//             out_string.append("3001");
//             break;
//         default:
//             out_string.append(CppString::ToString(cmd));
//             break;
//         }
// 
//         return 1 + digit_count;
//     }
// 
//     return -1;
// }
// 
// int32_t ParseTemplate(const std::string &template_string,
//                       std::string &out_string)
// {
//     out_string.clear();
//     int32_t ret;
// 
//     std::string::size_type last_pos = 0;
//     std::string::size_type pos = template_string.find("%", last_pos);
//     while (pos != std::string::npos)
//     {
//         out_string.append(template_string.data() + last_pos, pos - last_pos);
// 
//         if (++pos == template_string.size())
//         {
//             return -1;
//         }
// 
//         // 解析单个转义字符串，it指向%后面的一个字符
//         ret = GetEscape(template_string, pos, out_string);
//         if (ret <= 0)
//         {
//             return ret;
//         }
// 
//         last_pos = pos + ret;
//         pos = template_string.find("%", last_pos);
//     }
// 
//     out_string.append(template_string.data() + last_pos,
//                       template_string.size() - last_pos);
// 
//     return 0;
// }
// 
// TEST(string, temp)
// {
//     std::string out_string;
// 
//     EXPECT_EQ(0, ParseTemplate("123123", out_string));
//     EXPECT_EQ("123123", out_string);
// 
//     EXPECT_EQ(0, ParseTemplate("123%%123", out_string));
//     EXPECT_EQ("123%123", out_string);
// 
//     EXPECT_EQ(-1, ParseTemplate("123%%%523", out_string));
// 
//     EXPECT_EQ(0, ParseTemplate("123%%%%123", out_string));
//     EXPECT_EQ("123%%123", out_string);
// 
//     EXPECT_EQ(0, ParseTemplate("123%11123", out_string));
//     EXPECT_EQ("12311123", out_string);
// 
//     EXPECT_EQ(0, ParseTemplate("123%%11123", out_string));
//     EXPECT_EQ("123%11123", out_string);
// 
//     EXPECT_EQ(0, ParseTemplate("123%41123", out_string));
//     EXPECT_EQ("12341123", out_string);
// 
//     EXPECT_EQ(-1, ParseTemplate("123%51123", out_string));
// 
//     EXPECT_EQ(0, ParseTemplate("%123%%11123", out_string));
//     EXPECT_EQ("123%11123", out_string);
// 
//     EXPECT_EQ(-1, ParseTemplate("%123%%11123%", out_string));
// 
//     EXPECT_EQ(0, ParseTemplate("%123%%11123%%", out_string));
//     EXPECT_EQ("123%11123%", out_string);
// }
