#include <stdint.h>

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <iterator>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <ext/slist>

#include "gtest/gtest.h"

#include <CppArray.h>
#include <CppTime.h>
#include <CppString.h>

#include "global.h"

using namespace std;
using namespace __gnu_cxx;

static uint32_t MAX_NUM = 10000000;

TEST(CppSTL, FindTest)
{
    const string NAME[] = {"Adam", "Hello", "Moon"};

    EXPECT_FALSE(find(NAME, NAME + ARRAY_SIZE(NAME), "Hello") == NAME + ARRAY_SIZE(NAME));
    EXPECT_TRUE(find(NAME, NAME + ARRAY_SIZE(NAME), "No Name") == NAME + ARRAY_SIZE(NAME));
}

/*
测试结果

Ubuntu虚拟机
MAX_NUM is 10000000
[2014-06-01 22:34:43.935509]Begin insert 10000000 items to map:iMap[i]=i
[2014-06-01 22:34:49.807607]Finish insert 10000000 items to map:iMap[i]=i: From start 5872098 us,from last 5872098 us
[2014-06-01 22:34:49.807772]Begin find item in 10000000 map:iMap[MAX_NUM / 2]
[2014-06-01 22:34:49.807797]Finish find item in 10000000 map:iMap[MAX_NUM / 2]: From start 25 us,from last 25 us
[2014-06-01 22:34:50.145176]Begin insert 10000000 items to map:iMap.insert(pair)
[2014-06-01 22:34:55.687631]Finish insert 10000000 items to map:iMap.insert(pair): From start 5542455 us,from last 5542455 us
[2014-06-01 22:34:55.687739]Begin find item in 10000000 map:iMap.find(MAX_NUM / 2)
[2014-06-01 22:34:55.687774]Finish find item in 10000000 map:iMap.find(MAX_NUM / 2): From start 35 us,from last 35 us
[2014-06-01 22:34:55.687836]Begin size in 10000000 map:iMap.size()
[2014-06-01 22:34:55.687853]Finish size in 10000000 map:iMap.size(): From start 17 us,from last 17 us
[2014-06-01 22:34:55.687869]Begin insert 10000000 items to unordered_map:iHashMap[i]=i
[2014-06-01 22:34:56.418945]Finish insert 10000000 items to unordered_map:iHashMap[i]=i: From start 731076 us,from last 731076 us
[2014-06-01 22:34:56.419053]Begin find item in 10000000 unordered_map:iHashMap[MAX_NUM / 2]
[2014-06-01 22:34:56.419073]Finish find item in 10000000 unordered_map:iHashMap[MAX_NUM / 2]: From start 20 us,from last 20 us
[2014-06-01 22:34:56.633673]Begin insert 10000000 items to unordered_map:iHashMap.insert(pair)
[2014-06-01 22:34:57.117382]Finish insert 10000000 items to unordered_map:iHashMap.insert(pair): From start 483709 us,from last 483709 us
[2014-06-01 22:34:57.117490]Begin find item in 10000000 unordered_map:iHashMap.find(MAX_NUM / 2)
[2014-06-01 22:34:57.117512]Finish find item in 10000000 unordered_map:iHashMap.find(MAX_NUM / 2): From start 22 us,from last 22 us
[2014-06-01 22:34:57.117529]Begin size in 10000000 unordered_map:iHashMap.size()
[2014-06-01 22:34:57.117585]Finish size in 10000000 unordered_map:iHashMap.size(): From start 56 us,from last 56 us
[2014-06-01 22:34:57.117604]Begin insert 10000000 items to set
[2014-06-01 22:35:01.698215]Finish insert 10000000 items to set: From start 4580611 us,from last 4580611 us
[2014-06-01 22:35:01.698315]Begin find item in 10000000 set:iSet.find(MAX_NUM / 2)
[2014-06-01 22:35:01.698340]Finish find item in 10000000 set:iSet.find(MAX_NUM / 2): From start 25 us,from last 25 us
[2014-06-01 22:35:01.698356]Begin size in 10000000 set:iSet.size()
[2014-06-01 22:35:01.698371]Finish size in 10000000 set:iSet.size(): From start 15 us,from last 15 us
[2014-06-01 22:35:01.698386]Begin insert 10000000 items to unordered_set
[2014-06-01 22:35:02.491090]Finish insert 10000000 items to unordered_set: From start 792704 us,from last 792704 us
[2014-06-01 22:35:02.491191]Begin find item in 10000000 unordered_set:iHashSet.find(MAX_NUM / 2)
[2014-06-01 22:35:02.491211]Finish find item in 10000000 unordered_set:iHashSet.find(MAX_NUM / 2): From start 20 us,from last 20 us
[2014-06-01 22:35:02.491228]Begin size in 10000000 unordered_set:iHashSet.size()
[2014-06-01 22:35:02.491241]Finish size in 10000000 unordered_set:iHashSet.size(): From start 13 us,from last 13 us
[2014-06-01 22:35:02.491256]Begin insert 10000000 items to vector:iVector.push_back(i)
[2014-06-01 22:35:02.613971]Finish insert 10000000 items to vector:iVector.push_back(i): From start 122715 us,from last 122715 us
[2014-06-01 22:35:02.614078]Begin find item in 10000000 vector:find(iVector.begin(), iVector.end(), MAX_NUM / 2)
[2014-06-01 22:35:02.619124]Finish find item in 10000000 vector:find(iVector.begin(), iVector.end(), MAX_NUM / 2): From start 5046 us,from last 5046 us
[2014-06-01 22:35:02.619779]Begin size in 10000000 vector:iVector.size()
[2014-06-01 22:35:02.619980]Finish size in 10000000 vector:iVector.size(): From start 201 us,from last 201 us
[2014-06-01 22:35:02.620071]Begin insert 10000000 items to list:iList.push_back(i)
[2014-06-01 22:35:03.139080]Finish insert 10000000 items to list:iList.push_back(i): From start 519009 us,from last 519009 us
[2014-06-01 22:35:03.139183]Begin find item in 10000000 list:find(iList.begin(), iList.end(), MAX_NUM / 2)
[2014-06-01 22:35:03.154212]Finish find item in 10000000 list:find(iList.begin(), iList.end(), MAX_NUM / 2): From start 15029 us,from last 15029 us
[2014-06-01 22:35:03.154330]Begin size in 10000000 list:iList.size()
[2014-06-01 22:35:03.181290]Finish size in 10000000 list:iList.size(): From start 26960 us,from last 26960 us
*/
TEST(CppSTL, PerformTest)
{
    bool skipTest = true;
    if (skipTest)
    {
        cout << "Skip PerformTest" << endl;
        return;
    }

    cout << "MAX_NUM is " << MAX_NUM << endl;

    map<uint32_t, uint32_t> iMap;
    set<uint32_t> iSet;
    vector<uint32_t> iVector;
    list<uint32_t> iList;
    unordered_map<uint32_t, uint32_t> iHashMap;
    unordered_set<uint32_t> iHashSet;

    string timerMsg;
    CppShowTimer timer;

    /************************************************************************/
    /* map                                                                  */
    /************************************************************************/
    /* 直接索引添加到map */
    iMap.clear();
    timerMsg = CppString::GetArgs(" insert %u items to map:iMap[i]=i", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iMap[i] = i;
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接索引查找map */
    timerMsg = CppString::GetArgs(" find item in %u map:iMap[MAX_NUM / 2]", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iMap[MAX_NUM / 2];
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 使用insert添加到map */
    iMap.clear();
    timerMsg = CppString::GetArgs(" insert %u items to map:iMap.insert(pair)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iMap.insert(pair<uint32_t, uint32_t>(i, i));
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接find查找map */
    timerMsg = CppString::GetArgs(" find item in %u map:iMap.find(MAX_NUM / 2)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iMap.find(MAX_NUM / 2);
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 求map的size */
    timerMsg = CppString::GetArgs(" size in %u map:iMap.size()", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iMap.size();
    cout << timer.Record("Finish" + timerMsg) << endl;

    /************************************************************************/
    /* unordered_map                                                             */
    /************************************************************************/
    /* 直接索引添加到unordered_map */
    iHashMap.clear();
    timerMsg = CppString::GetArgs(" insert %u items to unordered_map:iHashMap[i]=i", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iHashMap[i] = i;
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接索引查找unordered_map */
    timerMsg = CppString::GetArgs(" find item in %u unordered_map:iHashMap[MAX_NUM / 2]", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iHashMap[MAX_NUM / 2];
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 使用insert添加到unordered_map */
    iHashMap.clear();
    timerMsg = CppString::GetArgs(" insert %u items to unordered_map:iHashMap.insert(pair)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iHashMap.insert(pair<uint32_t, uint32_t>(i, i));
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接find查找unordered_map */
    timerMsg = CppString::GetArgs(" find item in %u unordered_map:iHashMap.find(MAX_NUM / 2)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iHashMap.find(MAX_NUM / 2);
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 求unordered_map的size */
    timerMsg = CppString::GetArgs(" size in %u unordered_map:iHashMap.size()", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iHashMap.size();
    cout << timer.Record("Finish" + timerMsg) << endl;

    /************************************************************************/
    /* set                                                                  */
    /************************************************************************/
    /* 添加到set */
    iSet.clear();
    timerMsg = CppString::GetArgs(" insert %u items to set", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iSet.insert(i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接find查找set */
    timerMsg = CppString::GetArgs(" find item in %u set:iSet.find(MAX_NUM / 2)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iSet.find(MAX_NUM / 2);
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 求set的size */
    timerMsg = CppString::GetArgs(" size in %u set:iSet.size()", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iSet.size();
    cout << timer.Record("Finish" + timerMsg) << endl;

    /************************************************************************/
    /* unordered_set                                                             */
    /************************************************************************/
    /* 添加到unordered_set */
    iHashSet.clear();
    timerMsg = CppString::GetArgs(" insert %u items to unordered_set", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iHashSet.insert(i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接find查找unordered_set */
    timerMsg = CppString::GetArgs(" find item in %u unordered_set:iHashSet.find(MAX_NUM / 2)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iHashSet.find(MAX_NUM / 2);
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 求unordered_set的size */
    timerMsg = CppString::GetArgs(" size in %u unordered_set:iHashSet.size()", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iHashSet.size();
    cout << timer.Record("Finish" + timerMsg) << endl;

    /************************************************************************/
    /* vector                                                               */
    /************************************************************************/
    /* 添加到vector */
    iVector.clear();
    timerMsg = CppString::GetArgs(" insert %u items to vector:iVector.push_back(i)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iVector.push_back(i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接find查找vector */
    timerMsg = CppString::GetArgs(" find item in %u vector:find(iVector.begin(), iVector.end(), MAX_NUM / 2)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    find(iVector.begin(), iVector.end(), MAX_NUM / 2);
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 求vector的size */
    timerMsg = CppString::GetArgs(" size in %u vector:iVector.size()", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iVector.size();
    cout << timer.Record("Finish" + timerMsg) << endl;

    /************************************************************************/
    /* list                                                                 */
    /************************************************************************/
    /* 添加到list */
    iList.clear();
    timerMsg = CppString::GetArgs(" insert %u items to list:iList.push_back(i)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; ++i)
    {
        iList.push_back(i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 直接find查找list */
    timerMsg = CppString::GetArgs(" find item in %u list:find(iList.begin(), iList.end(), MAX_NUM / 2)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    find(iList.begin(), iList.end(), MAX_NUM / 2);
    cout << timer.Record("Finish" + timerMsg) << endl;

    /* 求list的size */
    timerMsg = CppString::GetArgs(" size in %u list:iList.size()", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    iList.size();
    cout << timer.Record("Finish" + timerMsg) << endl;
}

/*
测试结果

Ubuntu虚拟机
MAX_NUM is 10000000
[2014-11-12 10:20:15.620222]Begin append 10000000 char with string::append(1,i)
[2014-11-12 10:20:15.730999]Finish append 10000000 char with string::append(1,i): From start 110777 us,from last 110777 us
[2014-11-12 10:20:15.731313]Begin append 10000000 char with string::append(10,i)
[2014-11-12 10:20:15.759857]Finish append 10000000 char with string::append(10,i): From start 28544 us,from last 28544 us
[2014-11-12 10:20:15.760970]Begin append 10000000 char with string::append(100,i)
[2014-11-12 10:20:15.781739]Finish append 10000000 char with string::append(100,i): From start 20769 us,from last 20769 us
[2014-11-12 10:20:15.785466]Begin append 10000000 char with string+=i
[2014-11-12 10:20:15.838962]Finish append 10000000 char with string+=i: From start 53496 us,from last 53496 us
[2014-11-12 10:20:15.840081]Begin append char * with +=(char *)
[2014-11-12 10:20:16.073482]Finish append char * with +=(char *): From start 233401 us,from last 233401 us
[2014-11-12 10:20:16.074175]Begin append char * with string.append(char *)
[2014-11-12 10:20:16.316649]Finish append char * with string.append(char *): From start 242474 us,from last 242474 us
*/
TEST(CppSTL, StringPerformTest)
{
    bool skipTest = true;
    if (skipTest)
    {
        cout << "Skip StringPerformTest" << endl;
        return;
    }

    cout << "MAX_NUM is " << MAX_NUM << endl;

    string timerMsg;
    CppShowTimer timer;

    string iString;
    /* Append和+对比 */

    // 使用append(1,i)
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append %u char with string::append(1,i)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i++)
    {
        iString.append(1, (char)i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    // 使用append(10,i)
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append %u char with string::append(10,i)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i += 10)
    {
        iString.append(10, (char)i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    // 使用append(100,i)
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append %u char with string::append(100,i)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i += 100)
    {
        iString.append(100, (char)i);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    // 使用+=i
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append %u char with string+=i", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i++)
    {
        iString += (char)i;
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    const char *tempStr = "tempStr";

    // +=(char *)
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append char * with +=(char *)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i++)
    {
        iString += tempStr;
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    // append(char *)
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append char * with string.append(char *)", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i++)
    {
        iString.append(tempStr);
    }
    cout << timer.Record("Finish" + timerMsg) << endl;

    // 使用string=string+i
    string().swap(iString);
    timerMsg = CppString::GetArgs(" append %u char with string=string+i", MAX_NUM);
    cout << timer.Start("Begin" + timerMsg) << endl;
    for (uint32_t i = 0; i < MAX_NUM; i++)
    {
        iString = iString + (char)i;
    }
    cout << timer.Record("Finish" + timerMsg) << endl;
}

static void ShowVectorInfo(vector<int32_t> &c, const string &infoStr)
{
    const string SPLIT_LINE = string(80, '-');
    const string NAME = "vector";

    cout << SPLIT_LINE << endl;

    cout << CppString::GetArgs("[%s]%s:", NAME.c_str(), infoStr.c_str());
    copy(c.begin(), c.end(), ostream_iterator<int32_t>(cout, " "));
    cout << endl;

    cout << CppString::GetArgs("[%s]v.empty() = %d\n", NAME.c_str(), c.empty());
    cout << CppString::GetArgs("[%s]v.size() = %llu\n", NAME.c_str(), c.size());
    cout << CppString::GetArgs("[%s]v.capacity() = %llu\n", NAME.c_str(), c.capacity());
    cout << CppString::GetArgs("[%s]v.max_size() = %llu\n", NAME.c_str(), c.max_size());

    cout << SPLIT_LINE << endl;
}

TEST(CppSTL, vector)
{
    const string NAME = "vector";

    /* 定义容器 */
    vector<int32_t> v;
    ShowVectorInfo(v, "Empty");

    /* 插入数据 */
    v.push_back(1);
    v.push_back(3);
    v.push_back(5);
    v.push_back(7);
    v.push_back(9);
    ShowVectorInfo(v, "Insert data");

    /* 数据操作 */
    cout << CppString::GetArgs("[%s]v.front() = %d\n", NAME.c_str(), v.front());
    cout << CppString::GetArgs("[%s]v.back() = %d\n", NAME.c_str(), v.back());
    cout << CppString::GetArgs("[%s]v.at(1) = %d\n", NAME.c_str(), v.at(1));
    cout << CppString::GetArgs("[%s]*v.data() = %d\n", NAME.c_str(), *v.data());

    /* 删除数据 */
    v.pop_back();
    ShowVectorInfo(v, "v.pop_back()");

    // 删除指定前向迭代器位置的元素
    v.erase(v.begin());
    ShowVectorInfo(v, "v.erase(v.begin())");

    /* 大小调整 */
    // 如果传入的值大于当前size，则在末尾补默认值
    v.resize(10);
    ShowVectorInfo(v, "v.resize(10)");

    // 删除指定值的元素
    v.erase(remove(v.begin(), v.end(), 3), v.end());
    ShowVectorInfo(v, "v.erase(remove(v.begin(), v.end(), 3), v.end())");

    // 删除满足一定条件的元素，比如删除大于5的元素
    v.erase(remove_if(v.begin(), v.end(), bind2nd(greater<int32_t>(), 5)), v.end());
    ShowVectorInfo(v, "v.erase(remove_if(v.begin(), v.end(), bind2nd(greater<int32_t>(), 5)), v.end())");

    // 如果传入的值大于当前capacity，则扩容
    v.reserve(20);
    ShowVectorInfo(v, "v.reserve(20)");

    // 如果传入的值小于当前size，则删除末尾的值
    v.resize(4);
    ShowVectorInfo(v, "v.resize(4)");

    // 如果传入的值小于实际capacity，则值无效
    v.reserve(3);
    ShowVectorInfo(v, "v.reserve(3)");

    v.clear();
    ShowVectorInfo(v, "v.clear()");

    vector<int32_t>().swap(v);
    ShowVectorInfo(v, "vector<int32_t>().swap(v)");
}

TEST(CppSTL, deque)
{
    deque<int32_t> d;
}

void ShowListInfo(list<int32_t> &c, const string &infoStr)
{
    const string SPLIT_LINE = string(80, '-');
    const string NAME = "list";

    cout << SPLIT_LINE << endl;

    cout << CppString::GetArgs("[%s]%s:", NAME.c_str(), infoStr.c_str());
    copy(c.begin(), c.end(), ostream_iterator<int32_t>(cout, " "));
    cout << endl;

    cout << CppString::GetArgs("[%s]v.empty() = %d\n", NAME.c_str(), c.empty());
    cout << CppString::GetArgs("[%s]v.size() = %llu\n", NAME.c_str(), c.size());
    cout << CppString::GetArgs("[%s]v.max_size() = %llu\n", NAME.c_str(), c.max_size());

    cout << SPLIT_LINE << endl;
}

TEST(CppSTL, list)
{
    const string NAME = "list";

    list<int32_t> l1;
    list<int32_t> l2;

    ShowListInfo(l1, "Empty l1");

    l1.push_back(1);
    l1.push_back(3);
    l1.push_back(5);
    l1.push_back(7);
    l1.push_back(9);
    ShowListInfo(l1, "After insert l1");

    l2.push_back(2);
    l2.push_back(4);
    l2.push_back(6);
    l2.push_back(8);
    ShowListInfo(l2, "After insert l2");

    /* 数据操作 */
    cout << CppString::GetArgs("[%s]l1.front() = %d\n", NAME.c_str(), l1.front());
    cout << CppString::GetArgs("[%s]l1.back() = %d\n", NAME.c_str(), l1.back());

    /* 链表合并，要求2个链表都为有序，合并后l2被清空 */
    l1.sort();
    l2.sort();
    l1.merge(l2);
    ShowListInfo(l1, "l1.merge(l2),l1");
    ShowListInfo(l2, "l1.merge(l2),l2");

    /* 链表排序，制定排序函数 */
    l1.sort(greater<int32_t>());
    ShowListInfo(l1, "l1.sort(greater<int32_t>()),l1");

    /* 链表splice，将另一个链表的一些节点插入到本链表的指定位置并删除原节点 */
    l2.push_back(11);
    l2.push_back(13);
    l2.push_back(15);
    l1.splice(l1.begin(), l2);
    ShowListInfo(l1, "l1.splice(l1.begin(), l2),l1");
    ShowListInfo(l2, "l1.splice(l1.begin(), l2),l2");

    /* 删除元素 */
    // 删除指定值的元素
    l1.remove(2);
    ShowListInfo(l1, "l1.remove(2)");

    // 增加元素以备后用
    l1.push_back(10);
    l1.push_back(10);
    l1.push_back(10);
    l1.push_back(10);
    l1.push_back(10);

    // 删除满足一定条件的元素，下面示例表示删除小于8的元素
    l1.remove_if(bind2nd(less<int32_t>(), 8));
    ShowListInfo(l1, "l1.remove_if(bind2nd(less<int32_t>(), 8))");

    // 删除重复元素，需要排序
    l1.sort();
    l1.unique();
    ShowListInfo(l1, "l1.unique()");

    // 清空
    l1.clear();
    ShowListInfo(l1, "l1.clear()");

    l2.push_back(1);
    list<int32_t>().swap(l2);
    ShowListInfo(l2, "list<int32_t>().swap(l2)");
}

TEST(CppSTL, slist)
{
    slist<int32_t> s;
}

class UnorderedSetTest
{
public:
    static uint32_t constructCount;
    static uint32_t deconstructCount;
    uint32_t id;

    UnorderedSetTest(uint32_t inId) :id(inId)
    {
        ++constructCount;
        // INFOR_LOG("%s:%u", __FUNCTION__, constructCount);
    }

    UnorderedSetTest(const UnorderedSetTest &right)
    {
        ++constructCount;
        // INFOR_LOG("%s:%u", __FUNCTION__, constructCount);
        id = right.id;
    }

    UnorderedSetTest(UnorderedSetTest &&right)
    {
        ++constructCount;
        // INFOR_LOG("%s:%u", __FUNCTION__, constructCount);
        id ^= right.id;
        right.id ^= id;
        id ^= right.id;
    }

    bool operator==(const UnorderedSetTest &right) const
    {
        return id == right.id;
    }

    ~UnorderedSetTest()
    {
        ++deconstructCount;
        // INFOR_LOG("%s:%u", __FUNCTION__, deconstructCount);
    }
};

uint32_t UnorderedSetTest::constructCount;
uint32_t UnorderedSetTest::deconstructCount;

// 特化hash类
namespace std
{
template <>
struct hash< UnorderedSetTest>
{
    size_t operator()(const UnorderedSetTest& k) const
    {
        // 直接使用内置的hash函数
        return hash<int>()(k.id);
    }
};
}

TEST(CppSTL, pair)
{
    // pair<T1, T2>的大小比较，优先比较T1，在T1相同的时候，会比较T2。
    static uint32_t COUNT = 100;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        for (uint32_t j = 0; j < COUNT; ++j)
        {
            auto a = make_pair(i, j);
            auto b = make_pair(j, i);
            auto c = make_pair(i, i);
            auto d = make_pair(j, j);

            EXPECT_EQ(i < j, a < b);
            EXPECT_EQ(i == j, a == b);
            EXPECT_EQ(i > j, a > b);

            EXPECT_EQ(j < i, a < c);
            EXPECT_EQ(i == j, a == c);
            EXPECT_EQ(j > i, a > c);

            EXPECT_EQ(i < j, a < d);
            EXPECT_EQ(i == j, a == d);
            EXPECT_EQ(i > j, a > d);
        }
    }
}

TEST(CppSTL, unordered_set)
{
    unordered_set<int32_t> h;
    const uint32_t COUNT = 10000;
    uint32_t lastSize = 0;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        h.insert(i);
        if (lastSize != h.bucket_count())
        {
            cout << CppString::GetArgs("len[%u],bucket_count[%u]", i, h.bucket_count()) << endl;
            lastSize = h.bucket_count();
        }
    }

    // 判断在扩容时是否会因为数据量增加而复制内部的对象
    // 结论：不会
    unordered_set<UnorderedSetTest> unorderedSetTestSet;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        if (i % 2 == 0)
        {
            // 这种会调用转移构造函数
            unorderedSetTestSet.insert(UnorderedSetTest(i));
        }
        else
        {
            // 这样种会调用拷贝构造函数
            UnorderedSetTest a(i);
            unorderedSetTestSet.insert(a);
        }
    }

    // INFOR_LOG("COUNT[%u].", UnorderedSetTest::count);
    EXPECT_EQ(COUNT * 2, UnorderedSetTest::constructCount);

    // 析构函数调用COUNT次，还有COUNT个在unorderedSetTestSet中
    EXPECT_EQ(COUNT, UnorderedSetTest::deconstructCount);

    // 继续析构
    unorderedSetTestSet.clear();
    EXPECT_EQ(COUNT * 2, UnorderedSetTest::deconstructCount);
}

TEST(CppSTL, stack)
{
    stack<int32_t> p;
    p.push(5);
    p.push(4);
    p.push(7);
    p.push(3);
    p.push(8);
    p.push(6);
    p.push(2);
    p.push(9);
    p.push(1);

    while (!p.empty())
    {
        cout << p.top() << " ";
        p.pop();
    }

    cout << endl;
}

TEST(CppSTL, queue)
{
    queue<int32_t> p;
    p.push(5);
    p.push(4);
    p.push(7);
    p.push(3);
    p.push(8);
    p.push(6);
    p.push(2);
    p.push(9);
    p.push(1);

    while (!p.empty())
    {
        cout << p.front() << " ";
        p.pop();
    }

    cout << endl;
}

TEST(CppSTL, priority_queue)
{
    priority_queue<int32_t> p;
    p.push(5);
    p.push(4);
    p.push(7);
    p.push(3);
    p.push(8);
    p.push(6);
    p.push(2);
    p.push(9);
    p.push(1);

    while (!p.empty())
    {
        cout << p.top() << " ";
        p.pop();
    }

    cout << endl;
}

TEST(CppSTL, iter_swap)
{
    vector<int> v;
    vector<int> v2;

    v.push_back(2);
    v.push_back(3);

    v2.push_back(7);
    v2.push_back(8);
    v2.push_back(9);

    iter_swap(v.begin(), v2.begin());

    copy(v.begin(), v.end(), ostream_iterator<int32_t>(cout, " "));
    cout << endl;

    copy(v2.begin(), v2.end(), ostream_iterator<int32_t>(cout, " "));
    cout << endl;
}

TEST(CppSTL, replace_copy)
{
    vector<int> v;

    v.push_back(3);
    v.push_back(5);
    v.push_back(8);

    vector<int> v2(v.size());

    replace_copy(v.begin(), v.end(), v2.begin(), 5, 6);

    copy(v.begin(), v.end(), ostream_iterator<int32_t>(cout, " "));
    cout << endl;

    copy(v2.begin(), v2.end(), ostream_iterator<int32_t>(cout, " "));
    cout << endl;
}

// 有序容器的操作
TEST(CppSTL, sorted_vector)
{
    vector<int> v;
    static uint32_t COUNT = 10;

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        v.push_back(i * 10);
    }

    // v = [0, 10, 20, 30, 40, 50, 60, 70, 80, 90]

    // lower_bound：获得有序序列的下边界，寻找到第一个不小于value的值位置，即返回>=value的第一个位置，找不到则返回end()
    EXPECT_EQ(0, *lower_bound(v.begin(), v.end(), 0));
    EXPECT_EQ(10, *lower_bound(v.begin(), v.end(), 1));
    EXPECT_EQ(50, *lower_bound(v.begin(), v.end(), 50));
    EXPECT_EQ(60, *lower_bound(v.begin(), v.end(), 51));
    EXPECT_EQ(90, *lower_bound(v.begin(), v.end(), 90));
    EXPECT_EQ(v.end(), lower_bound(v.begin(), v.end(), 100));
    EXPECT_EQ(v.end(), lower_bound(v.begin(), v.end(), 1000));

    // upper_bound：获得有序序列的上边界，寻找到第一个大于value的值位置，即返回>value的第一个位置，找不到则返回end()
    EXPECT_EQ(0, *upper_bound(v.begin(), v.end(), -1));
    EXPECT_EQ(v.begin(), upper_bound(v.begin(), v.end(), -1));
    EXPECT_EQ(10, *upper_bound(v.begin(), v.end(), 0));
    EXPECT_EQ(10, *upper_bound(v.begin(), v.end(), 1));
    EXPECT_EQ(60, *upper_bound(v.begin(), v.end(), 50));
    EXPECT_EQ(60, *upper_bound(v.begin(), v.end(), 51));
    EXPECT_EQ(v.end(), upper_bound(v.begin(), v.end(), 90));
    EXPECT_EQ(v.end(), upper_bound(v.begin(), v.end(), 100));
    EXPECT_EQ(v.end(), upper_bound(v.begin(), v.end(), 1000));

    // binary_search：有序区间二分查找，存在返回true，不存在返回false
    EXPECT_TRUE(binary_search(v.begin(), v.end(), 0));
    EXPECT_FALSE(binary_search(v.begin(), v.end(), 1));
    EXPECT_TRUE(binary_search(v.begin(), v.end(), 50));
    EXPECT_FALSE(binary_search(v.begin(), v.end(), 51));
    EXPECT_TRUE(binary_search(v.begin(), v.end(), 90));
    EXPECT_FALSE(binary_search(v.begin(), v.end(), 100));
    EXPECT_FALSE(binary_search(v.begin(), v.end(), 1000));

    // equal_range：获得有序序列的等价区间，first是区间的起始，second是区间结束的下一位置
    // first与second相等，说明没找到，并且二者的值均指向大于value的下一位置
    auto result = equal_range(v.begin(), v.end(), 0);
    EXPECT_EQ(0, *result.first);
    EXPECT_EQ(10, *result.second);

    result = equal_range(v.begin(), v.end(), 1);
    EXPECT_TRUE(result.second == result.first);
    EXPECT_EQ(10, *result.first);

    result = equal_range(v.begin(), v.end(), 51);
    EXPECT_TRUE(result.second == result.first);
    EXPECT_EQ(60, *result.first);
}
