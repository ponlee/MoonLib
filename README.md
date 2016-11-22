# MoonLib

提供一些常用库的C++封装，主要目的是提高易用性。

* CppBigNum：大数处理（性能较差，做题用的）
* CppChartDir：图表库
* CppCrypto：MD5加密的简单封装
* CppCurl：libcurl的封装，处理HTTP协议的Get和Post协议（主要是方便写爬虫了）
* CppEnumType：一个枚举类型，支持字符串的转换
* CppEnv：应用程序环境相关信息的获取
* CppFile：文件的操作
* CppFraction：实现分数的计算（做OJ题用的）
* CppJson：jsoncpp库的封装，用于处理json
* CppLog：日志库（由于每一个日志都写磁盘，性能较差，但可优化，偶尔用用可以）
* CppLRU：一个固定容量LRU队列的实现
* CppMath：一些数学相关的操作（做OJ题用的）
* CppMysql：一个cmysql库的封装
* CppNet：网络相关功能
* CppRegex：pcre的封装，用于处理正则表达式
* CppString：字符串相关
* CppSystem：系统相关
* CppTime：时间相关
* CppTinyxml：TinyXML的封装
* CppZookeeper：Zookeeper C语言API的封装

## 使用方法
可以只提取自己需要的.cpp和.h文件即可，如果需要编译成库的话，需要自己去寻找相关的其他库，需要的库可以在makefile中找到，里面也可以定义库路径存放的路径，默认是../ext/include中存放头文件，../ext/libs中存放库文件。
一般建议使用软链接的方式进行第三方库的管理，这样升级库不需要改太多东西。

## 测试
测试用例在gtest目录中，编译运行即可，耗时较长的测试用例都加了DISABLED，不会默认运行。
Zookeeper相关的部分测试用例由于调用了系统命令iptables对网络进行阻断操作，因此需要root权限。
