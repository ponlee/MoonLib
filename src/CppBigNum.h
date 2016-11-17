#ifndef _CPP_BIG_NUM_H_
#define _CPP_BIG_NUM_H_

#include <stdint.h>
#include <string>
#include <ostream>

using std::string;
using std::ostream;

// 正整数大数运算
class CppBigNum
{
public:
    CppBigNum();
    CppBigNum(const string &num);
    CppBigNum(const char *num);
    CppBigNum(const uint64_t num);
    CppBigNum(const int64_t num);
    CppBigNum(const int32_t num);
    CppBigNum(const uint32_t num);
    ~CppBigNum();

    //************************************
    // Method:    Value
    // Access:    public
    // Describe:  获取大数的值
    // Returns:   string 
    //************************************
    string Value() const;

    //************************************
    // Method:    operator+
    // Access:    public
    // Describe:  大数相加
    // Parameter: const BigNum & right
    // Returns:   const BigNum
    //************************************
    const CppBigNum operator+(const CppBigNum &right) const;

    /** 大数相减
     *
     * @param 	const CppBigNum & right
     * @retval 	const CppBigNum
     * @author 	moontan
     */
    const CppBigNum operator-(const CppBigNum &right) const;

    //************************************
    // Method:    operator*
    // Access:    public 
    // Describe:  大数相乘
    // Parameter: const CppBigNum & right
    // Returns:   const CppBigNum
    //************************************
    const CppBigNum operator*(const CppBigNum &right) const;

    /** 大数相除，忽略余数
     *
     * @param 	const CppBigNum & right
     * @retval 	const CppBigNum
     * @author 	moontan
     */
    const CppBigNum operator/(const CppBigNum &right) const;

    //************************************
    // Method:    operator==
    // Access:    public
    // Describe:  判断2个大数是否相等
    // Parameter: const BigNum & right
    // Returns:   bool
    //************************************
    bool operator==(const CppBigNum &right) const;

    /** 实现!=
     *
     * @param 	const CppBigNum & right
     * @retval 	bool
     * @author 	moontan
     */
    bool operator!=(const CppBigNum &right) const;

    //************************************
    // Method:    operator<
    // Access:    public
    // Describe:  实现小于比较
    // Parameter: const BigNum & right
    // Returns:   bool
    //************************************
    bool operator<(const CppBigNum &right) const;

    /** 实现小于等于比较
     *
     * @param 	const CppBigNum & right
     * @retval 	bool
     * @author 	moontan
     */
    bool operator<=(const CppBigNum &right) const;

    //************************************
    // Describe:  赋值运算符重载
    // Parameter: const CppBigNum & right
    // Returns:   CppBigNum &
    // Author:    moontan
    //************************************
    CppBigNum &operator=(const CppBigNum &right);

    //************************************
    // Describe:  流输出运算符重载
    // Parameter: ostream & os
    // Parameter: const CppBigNum & right
    // Returns:   friend ostream&
    // Author:    moontan
    //************************************
    friend ostream& operator<<(ostream &os, const CppBigNum &right)
    {
        return os << right.Value();
    }

    //************************************
    // Method:    operator>
    // Access:    public
    // Describe:  实现大于比较
    // Parameter: const BigNum & right
    // Returns:   bool
    //************************************
    bool operator>(const CppBigNum &right) const;

    /** 实现大于等于比较
     *
     * @param 	const CppBigNum & right
     * @retval 	bool
     * @author 	moontan
     */
    bool operator>=(const CppBigNum &right) const;

    //************************************
    // Method:    operator++
    // Access:    public
    // Describe:  前自增,++value
    // Returns:   BigNum
    //************************************
    CppBigNum operator++();

    //************************************
    // Method:    operator++
    // Access:    public
    // Describe:  后自增,value++
    // Parameter: int
    // Returns:   BigNum
    //************************************
    CppBigNum operator++(int);

    //************************************
    // Method:    operator+=
    // Access:    public 
    // Describe:  重载+=
    // Parameter: CppBigNum & right
    // Returns:   CppBigNum
    //************************************
    CppBigNum operator+=(const CppBigNum &right);

    /** 重载-=
     *
     * @param 	const CppBigNum & right
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum operator-=(const CppBigNum &right);

    //************************************
    // Describe:  重载*=
    // Parameter: const CppBigNum &right
    // Returns:   CppBigNum
    // Author:    moontan
    //************************************
    CppBigNum operator*=(const CppBigNum &right);

    /** 重载/=
     *
     * @param 	const CppBigNum & right
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum operator/=(const CppBigNum &right);

    /** 重载%=
     *
     * @param 	const CppBigNum & right
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum operator%=(const CppBigNum &right);

    /** 重载取模
     *
     * @param 	const CppBigNum & right
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum operator%(const CppBigNum &right);

    //************************************
    // Method:    Reverse
    // Access:    public
    // Describe:  数字逆转,比如123变成321,但是2000会变成2
    // Returns:   BigNum
    //************************************
    CppBigNum Reverse() const;

    //************************************
    // Describe:  将给定num转换为整数格式，即仅包含0-9并且不以0开头，不能处理负数
    // Parameter: const string & num
    // Returns:   string
    // Author:    moontan
    //************************************
    void GetNumFromStr(const string &num);

    /** 取负值
     *
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum Negative() const;

    /** 求平方根，只能精确到整数级别，向下取整，非正数直接返回0
     *
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum Sqrt() const;

    /** 求绝对值
     *
     * @retval 	CppBigNum
     * @author 	moontan
     */
    CppBigNum Abs() const;

protected:
    string numStr;
    bool flag;      // 符号位，true表示>=0，false表示小于0

    /** 字符串表示的数字相比较，需要保证输入是规范的非负值数字，不包含以0或者符号开头的特殊数字
     *
     * @param 	const string & numStr1
     * @param 	const string & numStr2
     * @retval 	bool
     * @author 	moontan
     */
    static bool numStrLess(const string &numStr1, const string &numStr2);
};

#endif
