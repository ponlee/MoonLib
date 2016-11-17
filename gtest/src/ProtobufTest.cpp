#ifndef __CYGWIN__
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <iostream>
#include <string>
#include <sstream>

#include <boost/optional.hpp>

#include <CppString.h>

#include "gtest/gtest.h"

#include <protocol/protobufTest.pb.h>

using namespace std;
using namespace ProtobufTest;
using namespace google::protobuf;

TEST(Protobuf, SerializeAndParse)
{
    // 编码
    ProtobufTestRequest req;
    req.set_string_value("123");
    req.set_int32_value(-1);
    req.set_uint32_value(1);

    string encode = req.SerializeAsString();
    cout << CppString::Hex2Str(encode, true) << endl;

    // 解码
    ProtobufTestRequest req2;
    ProtobufTestRequest req3;
    EXPECT_TRUE(req2.ParseFromString(encode));

    EXPECT_EQ("123", req2.string_value());
    EXPECT_EQ(-1, req2.int32_value());
    EXPECT_EQ(1, req2.uint32_value());

    // 解码失败
    EXPECT_FALSE(req3.ParseFromString(string("1") + encode));
}

TEST(Protobuf, RepetedOperator)
{
    ProtobufTestResponse response;
    static const uint32_t COUNT = 100;

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        response.add_code(i);
    }

    // [0,99]
    EXPECT_EQ(COUNT, response.code_size());

    // [0,98]
    response.mutable_code()->RemoveLast();
    EXPECT_EQ(COUNT - 1, response.code_size());
    EXPECT_EQ(COUNT - 2, *(response.code().end() - 1));

    for (auto code_it = response.mutable_code()->begin(); code_it != response.mutable_code()->end(); ++code_it)
    {
        // 去除10的倍数
        if ((*code_it) % 10 == 0)
        {
            response.mutable_code()->SwapElements(code_it - response.mutable_code()->begin(),
                                                  response.mutable_code()->end() - response.mutable_code()->begin() - 1);

            response.mutable_code()->RemoveLast();
        }
    }

//     for (auto code_it = response.mutable_code()->begin(); code_it != response.mutable_code()->end(); ++code_it)
//     {
//         cout << *code_it << endl;
//     }
}

// 客户端
class ProtobufTestRpcChannel : public RpcChannel
{
    virtual void CallMethod(const MethodDescriptor* method,
                            RpcController* controller,
                            const Message* request,
                            Message* response,
                            Closure* done)
    {
        cout << __FUNCTION__ << endl;
    }
};

// 服务端
class ProtobufTestServiceImpl : public ProtobufTestService
{
    virtual void Echo(::google::protobuf::RpcController* controller,
                      const ::ProtobufTest::ProtobufTestRequest* request,
                      ::ProtobufTest::ProtobufTestResponse* response,
                      ::google::protobuf::Closure* done)
    {
        cout << __FUNCTION__ << endl;
    }
};

TEST(Protobuf, optional)
{
    ProtobufTestRequest req;
    boost::optional<int32_t> op_value = boost::make_optional(req.has_op_value(), req.op_value());
    EXPECT_EQ(3, op_value.get_value_or(3));
    EXPECT_FALSE(req.has_op_value());
}

#endif
