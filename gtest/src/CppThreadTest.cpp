#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>

#include "gtest/gtest.h"

using namespace std;

/************************************************************************/
/* 单生产者-多消费者模型                                                */
/************************************************************************/
static const uint64_t COUNT = 20000;        // 总共计数数量
static const uint64_t THREAD_COUNT = 2000;    // 线程数

uint64_t CurrValue;                         // 当前生产的值（生产者与消费者之间的消息传递），为0表示数据可以生产
uint64_t SumValue;                          // 消费者的输出值，所有生产值的和（多个消费者之间的消息传递）
bool Stop = false;                          // 是否停止
bool Start = false;                         // 是否开始

mutex ValueMutex;                           // 临界区互斥量
condition_variable SetCV;                   // 条件变量指示：是否可生产
condition_variable GetCV;                   // 条件变量指示：是否可读取

mutex DataMutex;                            // 消费者产出数据保护互斥量

static bool ShowProcInfo = false;           // 是否显示处理过程中的信息

// 生产者
void SetFun()
{
    for (uint64_t i = 1; i <= COUNT; ++i)
    {
        // 等待可生产信号
        unique_lock<mutex> setLock(ValueMutex);
        SetCV.wait(setLock, []
        {
            return CurrValue == 0;      // 等待，直到CurrValue为0，表示数据已经被取走
        });

        // 生产一个数据，将其存入CurrValue，供消费者读取
        CurrValue = i;

        if (ShowProcInfo)
        {
            cout << "生产" << i << "-----------" << endl;
        }
        // 通知一个消费者读取数据
        GetCV.notify_one();

        // setLock在此处析构会调用ValueMutex.unlock()解锁
    }

    // 设置停止标志
    Stop = true;

    // 通知所有的消费者，如果没有这一行，部分消费者会挂在wait中（通过GDB可以查看到）
    GetCV.notify_all();
}

// 消费者
void GetFun(uint32_t threadId)
{
    static_cast<void>(threadId);
    if (ShowProcInfo)
    {
        cout << "线程" << threadId << "启动" << endl;
    }
    uint64_t currValueBak;          // 取出生产者生产的数值临时存放，进行处理
    while (true)
    {
        unique_lock<mutex> getLock(ValueMutex);
        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "抢到锁" << endl;
        }
        Start = true;
        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "即将wait，释放锁" << endl;
        }
        GetCV.wait(getLock, []
        {
            return CurrValue != 0 || Stop;      // 仅当CurrValue不为0或者停止标志生效时，退出wait
        });

        // // 另一种等待形式：
        // while (CurrValue == 0 && !Stop)
        // {
        //     GetCV.wait(getLock);
        // }

        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "wait后,value=" << CurrValue << endl;
        }

        // 用于最后的退出部分
        if (CurrValue == 0 && Stop)
        {
            if (ShowProcInfo)
            {
                cout << "线程" << threadId << "break出去" << endl;
            }
            break;
        }

        // 记录数据后解锁，相当于提取数据
        currValueBak = CurrValue;

        // 设置标志
        CurrValue = 0;

        // 唤醒生产者
        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "通知生产者" << endl;
        }
        SetCV.notify_one();

        if (ShowProcInfo)
        {
            cout << "线程" << threadId << "释放锁，处理" << currValueBak << endl;
        }
        getLock.unlock();

        // 模拟处理数据延时，每个线程单独处理
        usleep(rand() % 20000);

        // 记录数据，需要对共享数据加锁
        unique_lock<mutex> dataLock(DataMutex);
        SumValue += currValueBak;
    }
}

TEST(CppThreadTest, DISABLED_ConditionVariable)
{
    vector<thread> threads;

    // 创建消费线程
    for (uint64_t i = 0; i < THREAD_COUNT; ++i)
    {
        threads.push_back(thread(GetFun, i));
    }

    // 等待信号启动生产者，因为无需太久，此处采用忙等待
    while (!Start);
    SetFun();

    // 等待消费者线程结束
    for (auto &currThread : threads)
    {
        currThread.join();
    }

    // 计算期望数值
    uint64_t expectValue = (1 + COUNT) * COUNT / 2;
    EXPECT_EQ(expectValue, SumValue);
}

TEST(CppThreadTest, Mutex)
{
    volatile int counter(0); // non-atomic counter
    std::mutex mtx;           // locks access to counter

    static const uint32_t THREAD_COUNT = 10;
    static const uint32_t COUNT = 10000;
    std::thread threads[THREAD_COUNT];
    for (uint32_t i = 0; i < THREAD_COUNT; ++i)
        threads[i] = std::thread([&]
    {
        for (uint32_t i = 0; i < COUNT; ++i)
        {
            lock_guard<mutex> lock(mtx);
            // only increase if currently not locked:
            ++counter;
        }
    });

    for (auto& th : threads) th.join();
    std::cout << counter << " successful increases of the counter.\n";
}
