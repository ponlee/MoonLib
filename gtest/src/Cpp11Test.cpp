#include <chrono>
#include <iostream>
#include <typeinfo>
#include <memory>

#include <cxxabi.h>
#ifndef __CYGWIN__
#include <execinfo.h>
#endif

#include "gtest/gtest.h"

#include <CppString.h>

#include "global.h"

using namespace std;
using namespace std::chrono;

TEST(Cpp11, chrono)
{
    duration<int32_t> d(50);
    EXPECT_EQ(50, d.count());
}

TEST(Cpp11, std_move)
{
    string a = "a";
    string b = "b";
    a = std::move(b);

    EXPECT_EQ("a", b);
    EXPECT_EQ("b", a);
}

/* 线程局部变量 */
thread_local uint32_t thread_param = 0;
TEST(Cpp11, thread)
{
    EXPECT_EQ(0u, thread_param);
}

class ClassName
{
public:
    string GetClassName()
    {
        const type_info &class_info = typeid(*this);
        int status;

        char *prealname = abi::__cxa_demangle(class_info.name(), 0, 0, &status);

        string name(prealname);
        free(prealname);

        return name;
    }

    static void fun()
    {
        cout << 1 << endl;
    }
};

void foo(void)
{
    printf("foo\n");
}

TEST(Cpp11, typeid)
{
    ClassName class_name;
    EXPECT_EQ("ClassName", class_name.GetClassName());
}

struct BindFuncTest : public binary_function < int, int, bool >
{
    bool operator()(int a, int b) const
    {
        return a < b;
    }
};

/* bind引用测试 */
class RefParam
{
public:
    static uint32_t Count;

    RefParam()
    {
        ++Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }

    RefParam(const RefParam &refParam)
    {
        static_cast<void>(refParam);
        ++Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }

    RefParam(RefParam &&refParam)
    {
        static_cast<void>(refParam);
        ++Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }

    ~RefParam()
    {
        --Count;
        DEBUG_LOG("[%s]Count[%u].", __FUNCTION__, Count);
    }
};

uint32_t RefParam::Count;

void FuncParam(RefParam refParam)
{
    static_cast<void>(refParam);
}

void FuncRefParam(RefParam &refParam)
{
    static_cast<void>(refParam);
}

void FuncRightRefParam(RefParam &&refParam)
{
    static_cast<void>(refParam);
}

TEST(Cpp11, bind)
{
    BindFuncTest test;
    EXPECT_FALSE(bind1st(test, 8)(3));          // 8 < 3
    EXPECT_TRUE(bind2nd(test, 8)(3));           // 3 < 8

    EXPECT_FALSE(bind1st(less<int>(), 8)(3));   // 8 < 3
    EXPECT_TRUE(bind2nd(less<int>(), 8)(3));    // 3 < 8

    EXPECT_EQ(0u, RefParam::Count);
    RefParam refParam;
    EXPECT_EQ(1u, RefParam::Count);
    auto funcParam = bind(FuncParam, refParam);
    EXPECT_EQ(2u, RefParam::Count);
    funcParam();
    auto funcParamWithRef = bind(FuncParam, ref(refParam));
    funcParamWithRef();
    EXPECT_EQ(2u, RefParam::Count);
}

class InitializeInClass
{
public:
    int32_t a = 1;
    int32_t b{3};
    static const int32_t aa = 3;

    InitializeInClass(int32_t inA) :a(inA)
    {

    }
};

TEST(Cpp11, initializeInClass)
{
    InitializeInClass c(4);
    EXPECT_EQ(4, c.a);
    EXPECT_EQ(3, c.b);
}

using Class1 = ClassName;

class Base
{
public:
    Base()
    {
    }

    virtual void fun() final
    {
    }

    virtual void fun2()
    {
    }

    virtual ~Base()
    {
    }
};

class Derived :public Base
{
public:
    virtual void fun2() override
    {
    }
};

class LiteralConst
{
public:
    int32_t a;
};

#ifndef __CYGWIN__
LiteralConst operator "" _C(const char *col, size_t n)
{
    static_cast<void>(n);
    LiteralConst literalConst;
    literalConst.a = CppString::FromString<int32_t>(col);
    return literalConst;
}

// 字面值常量
TEST(Cpp11, LiteralConst)
{
    EXPECT_EQ(123, ("123"_C).a);
}
#endif

template < unsigned N >
struct Fib
{
    enum
    {
        Val = Fib<N - 1>::Val + Fib<N - 2>::Val
    };
};

template<>    //针对和的特化作为结束的条件
struct Fib<0>
{
    enum
    {
        Val = 0
    };
};

template<>
struct Fib<1>
{
    enum
    {
        Val = 1
    };
};

TEST(Cpp11, meta)
{
    EXPECT_EQ(55, Fib<10>::Val);
}

TEST(Cpp11, shared_ptr_and_weak_ptr)
{
    shared_ptr<int> pInt = make_shared<int>(3);
    EXPECT_EQ(3, *pInt);
    EXPECT_EQ(3, *pInt.get());

    // reset接口：替换管理的指针
    pInt.reset(new int(5));
    EXPECT_EQ(5, *pInt);

    // use_count接口：判断有多少个引用
    // unique接口：判断是否是唯一拥有此内存的shared_ptr，比use_count快一点
    EXPECT_EQ(1, pInt.use_count());
    EXPECT_TRUE(pInt.unique());

    // 增加一个shared_ptr
    shared_ptr<int> pInt2(pInt);
    EXPECT_EQ(2, pInt.use_count());
    EXPECT_EQ(2, pInt2.use_count());
    EXPECT_FALSE(pInt.unique());
    EXPECT_FALSE(pInt2.unique());

    // owner_before：用于比较大小，相当于"<"符号，内部相同的shared_ptr进行比较结果为false
    EXPECT_FALSE(pInt.owner_before(pInt2));
    EXPECT_FALSE(pInt2.owner_before(pInt));

    // weak_ptr：弱引用，不计数，可用于观察
    weak_ptr<int> pWeakInt(pInt);

    // 弱引用不影响计数
    EXPECT_EQ(2, pInt.use_count());
    EXPECT_FALSE(pInt.unique());

    // use_count：weak_ptr也有
    EXPECT_EQ(2, pWeakInt.use_count());

    // expired：检查是否数据已经不存在了
    EXPECT_FALSE(pWeakInt.expired());

    // lock：创建一个shared_ptr出来，如果失败，则内部为NULL
    shared_ptr<int> pInt3(pWeakInt.lock());
    EXPECT_TRUE(pInt3 != NULL);             // lock后判空不可少，否则可能会产生空指针访问
    EXPECT_EQ(3, pWeakInt.use_count());

    // 全部释放
    pInt.reset();
    pInt2.reset();
    pInt3.reset();

    EXPECT_EQ(0, pWeakInt.use_count());
    EXPECT_TRUE(pWeakInt.expired());

    shared_ptr<int> pInt4 = pWeakInt.lock();
    EXPECT_FALSE(pInt3 != NULL);
}

// 测试完美转发
template<typename T>
struct TypeName
{
    static const char *get()
    {
        return "Type";
    }
};

template<typename T>
struct TypeName<const T>
{
    static const char *get()
    {
        return "const Type";
    }
};

template<typename T>
struct TypeName<T&>
{
    static const char *get()
    {
        return "Type&";
    }
};

template<typename T>
struct TypeName<const T&>
{
    static const char *get()
    {
        return "const Type&";
    }
};

template<typename T>
struct TypeName<T&&>
{
    static const char *get()
    {
        return "Type&&";
    }
};

template<typename T>
struct TypeName<const T&&>
{
    static const char *get()
    {
        return "const Type&&";
    }
};

template < typename T>
const char * printValType(T &&val)
{
    static_cast<void>(val);
    return TypeName<T&&>::get();
}

class A
{
};

// 左值
A &lRefA()
{
    static A a;
    return a;
}

// 常左值
const A& clRefA()
{
    static A a;
    return a;
}

// 右值
A rRefA()
{
    return A();
}

// 常右值
const A crRefA()
{
    return A();
}

template<typename T>
const char * funcWithRightValue(T &&a)
{
    return printValType(a);
}

template<typename T>
const char * funcWithRightValueWithForward(T &&a)
{
    return printValType(forward<T>(a));
}

TEST(Cpp11, forward)
{
    // 正常情况
    EXPECT_EQ("Type&", printValType(lRefA()));
    EXPECT_EQ("const Type&", printValType(clRefA()));
    EXPECT_EQ("Type&&", printValType(rRefA()));
    EXPECT_EQ("const Type&&", printValType(crRefA()));

    // 传入函数，右值会退化为左值
    EXPECT_EQ("Type&", funcWithRightValue(lRefA()));
    EXPECT_EQ("const Type&", funcWithRightValue(clRefA()));
    EXPECT_EQ("Type&", funcWithRightValue(rRefA()));            // 应该是Type&&
    EXPECT_EQ("const Type&", funcWithRightValue(crRefA()));     // 应该是const Type&&

    // 带forward的作用，完美转发
    EXPECT_EQ("Type&", funcWithRightValueWithForward(lRefA()));
    EXPECT_EQ("const Type&", funcWithRightValueWithForward(clRefA()));
    EXPECT_EQ("Type&&", funcWithRightValueWithForward(rRefA()));
    EXPECT_EQ("const Type&&", funcWithRightValueWithForward(crRefA()));
}
