#include "CppNet.h"

#ifndef __CYGWIN__
#include <sys/epoll.h>
#include <netdb.h>      // hostent
#include <fcntl.h>
#include <netinet/tcp.h>
#endif
#include <arpa/inet.h>

#include <list>
#include <thread>

using namespace std;

static const uint32_t MAX_IP_STR_LEN = 16;
static const uint32_t BUF_SIZE = 256;                       // 读写数据缓冲区大小

string CppNet::NetIpToStr(uint32_t ip)
{
    return inet_ntoa(*((in_addr *)&ip));
}

uint32_t CppNet::StrToNetIp(const string &ipStr)
{
    uint32_t ip;

    if (inet_aton(ipStr.c_str(), (in_addr *)&ip))
    {
        return ip;
    }

    THROW("StrToNetIp error,ip=%s", ipStr.c_str());
}

// bool CppNet::NetIsOK( vector<IpPort> &ipPort )
// {
//     FILE *fp;
//     for (vector<IpPort>::const_iterator it = ipPort.begin();it!=ipPort.end();++it)
//     {
//         if (!(fp=popen((string("ping -c 4 ")+it->IP).c_str(),"r")))
//         {
//             continue;
//         }
//     }
// }

#ifndef __CYGWIN__
CppEpollManager::CppEpollManager(uint32_t size) throw(CppException)
{
    mEpollFd = epoll_create(size);
    CHECK_THROW_F(mEpollFd >= 0, "epoll_create失败,errno[%d],error[%s].", errno, strerror(errno));
    mUniqEpollFd = make_shared<UniqueFd>(mEpollFd);
}

CppEpollManager::~CppEpollManager()
{

}

int32_t CppEpollManager::ProcWrite(epoll_event &inevent, function<int32_t(epoll_event &inevent)> &writeFunc)throw(CppException)
{
    int fd = GetFdFromEvent(inevent);

    int32_t ret = writeFunc(inevent);
    if (ret != 0)
    {
        return ret;
    }

    // 检查可读状态
    inevent.events = EPOLLIN | EPOLLRDHUP;
    AddOrModFd(fd, inevent);

    // DEBUG_LOG("Proc a request,send[%lu].", gClientData[inevent.data.fd]);

    return 0;
}

int32_t CppEpollManager::ProcRead(epoll_event &inevent, function<int32_t(epoll_event &inevent)> &readFunc) throw(CppException)
{
    int fd = GetFdFromEvent(inevent);

    // 读取请求
    int32_t ret = readFunc(inevent);
    if (ret != 0)
    {
        return ret;
    }

    // 检查可写状态
    inevent.events = EPOLLOUT | EPOLLRDHUP;
    AddOrModFd(fd, inevent);

    // DEBUG_LOG("Read data[%lu].", actualValue);

    return 0;
}

void CppEpollManager::AddOrModFd(int fd, epoll_event &event) throw(CppException)
{
    int32_t ret = 0;
    SetFdToEvent(fd, event);
    unique_lock<mutex> lock(mFdLock);
    if (mFds.find(fd) == mFds.end())
    {
        ret = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &event);
        mFds.insert(fd);
    }
    else
    {
        ret = epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, &event);
    }

    ERROR_THROW_F(ret, "epoll_ctl failed,event[%u],fd[%d],errno[%d],error[%s].",
                  event.events, fd, errno, strerror(errno));
}

void CppEpollManager::DelFd(int fd) throw(CppException)
{
    int32_t ret = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, NULL);
    ERROR_THROW_F(ret, "epoll_ctl失败,fd[%d],errno[%d],error[%s].", fd, errno, strerror(errno));
    unique_lock<mutex> lock(mFdLock);
    mFds.erase(fd);
    // DEBUG_LOG("Delete fd[%d] from epoll success.", fd);
}

void CppEpollManager::Wait(function<int32_t(epoll_event &inevent)> &readFunc,
                           function<int32_t(epoll_event &inevent)> &writeFunc,
                           function<void(int fd)> &deleteFdFunc,
                           uint32_t timeOutMs) throw(CppException)
{
    {
        unique_lock<mutex> lock(mFdLock);

        // mEvents扩容，缩容使用其他函数手工实现
        if (mEvents.size() < mFds.size())
        {
            mEvents.resize(mFds.size());
        }
    }

    int32_t fdsCount = epoll_wait(mEpollFd, &mEvents[0], mEvents.size(), timeOutMs);
    int32_t ret = 0;

    for (int32_t i = 0; i < fdsCount; ++i)
    {
        epoll_event &event = mEvents[i];
        if (event.events & EPOLLIN)
        {
            if (event.events & EPOLLRDHUP)
            {
                // 同时有EPOLLIN和EPOLLRDHUP事件表示对端断开，删除fd
                DelFd(GetFdFromEvent(event));
                if (deleteFdFunc)
                {
                    deleteFdFunc(event.data.fd);
                }
            }
            else
            {
                // 读取并且检查数据
                ret = ProcRead(event, readFunc);

                // 失败删除fd
                if (ret != 0)
                {
                    DelFd(GetFdFromEvent(event));
                    if (deleteFdFunc)
                    {
                        deleteFdFunc(event.data.fd);
                    }
                }
            }
        }
        else if (event.events & EPOLLOUT)
        {
            // 写入数据
            ret = ProcWrite(event, writeFunc);

            // 失败删除fd
            if (ret != 0)
            {
                DelFd(GetFdFromEvent(event));
                if (deleteFdFunc)
                {
                    deleteFdFunc(event.data.fd);
                }
            }
        }
        else
        {
            continue;
        }
    }
}

int32_t CppEpollManager::Wait(epoll_event events[], uint32_t eventSize, uint32_t timeOutMs)
{
    int32_t pollSize = epoll_wait(mEpollFd, events, eventSize, timeOutMs);
    return pollSize;
}

void CppEpollManager::ReleaseEventsMemory()
{
    mEvents.shrink_to_fit();
}

int CppEpollManager::GetFdFromEvent(epoll_event &event)
{
    return event.data.fd;
}

void CppEpollManager::SetFdToEvent(int fd, epoll_event &event)
{
    event.data.fd = fd;
}

MultiThreadClientBase::MultiThreadClientBase(const std::string &serverAddr, uint16_t serverPort,
                                             uint32_t runSecond, uint32_t clientThreadCount,
                                             uint32_t clientCountPerThread,
                                             uint32_t epollSize, CppLog *mpCppLog) :
    gSuccessCount(0), gFailCount(0), gTotalTimeUs(0), gMinTimeUs(0), gMaxTimeUs(0),
    gCurrTotalTimeUs(0), gCurrMinTimeUs(0), gCurrMaxTimeUs(0), gClientStop(false),
    mServerAddr(serverAddr), mServerPort(serverPort), mRunSecond(runSecond),
    mClientThreadCount(clientThreadCount), mClientCountPerThread(clientCountPerThread),
    mEpollSize(epollSize), mpCppLog(mpCppLog)
{

}

int32_t MultiThreadClientBase::ProcWrite(uint32_t threadId, epoll_event &inevent)
{
    const string data = GetSendData(threadId, inevent.data.fd);
    int32_t ret = write(inevent.data.fd, data.data(), data.size());
    CHECK_RETURN_F(mpCppLog, ret > 0, ret, CppLog::ERROR,
                   "write失败,event[%u],fd[%d],errno[%d],error[%s].",
                   inevent.events, inevent.data.fd, errno, strerror(errno));
    // DEBUG_LOG("Proc a request,send[%lu].", gClientData[inevent.data.fd]);

    // 记录开始时间
    gettimeofday(&GetClientData(threadId, inevent.data.fd)->sendTime, NULL);

    return 0;
}

int32_t MultiThreadClientBase::ProcRead(uint32_t threadId, epoll_event &inevent)
{
    // 记录结束时间
    timeval receiveTime;
    gettimeofday(&receiveTime, NULL);

    // 计算耗时
    int64_t usedTimeUs = CppTime::TimevDiff(receiveTime, GetClientData(threadId, inevent.data.fd)->sendTime);
    CHECK_RETURN_F(mpCppLog, usedTimeUs > 0, -1, CppLog::ERROR, "时间不正确[%ld].", usedTimeUs);

    // 总耗时统计
    gTotalTimeUs += usedTimeUs;
    if (gMaxTimeUs < static_cast<uint64_t>(usedTimeUs))
    {
        gMaxTimeUs = usedTimeUs;
    }

    if (gMinTimeUs == 0 || gMinTimeUs > static_cast<uint64_t>(usedTimeUs))
    {
        gMinTimeUs = usedTimeUs;
    }

    // 周期耗时统计
    gCurrTotalTimeUs += usedTimeUs;

    if (gCurrMaxTimeUs < static_cast<uint64_t>(usedTimeUs))
    {
        gCurrMaxTimeUs = usedTimeUs;
    }

    if (gCurrMinTimeUs == 0 || gCurrMinTimeUs > static_cast<uint64_t>(usedTimeUs))
    {
        gCurrMinTimeUs = usedTimeUs;
    }

    // 读取请求
    string bufStr;
    char buf[BUF_SIZE];

    int readSize;
    while ((readSize = read(inevent.data.fd, buf, sizeof(buf))) > 0)
    {
        CHECK_RETURN_F(mpCppLog, readSize > 0, readSize, CppLog::ERROR,
                       "read error,readSize[%d],event[%u],fd[%d,]errno[%d],error[%s].", readSize, inevent.events, inevent.data.fd, errno, strerror(errno));

        bufStr.append(buf, readSize);
    }

    // DEBUG_LOG("Read data[%lu].", actualValue);

    CheckResponse(threadId, inevent.data.fd, bufStr) ? ++gSuccessCount : ++gFailCount;

    return 0;
}

void MultiThreadClientBase::ProcDeleteFd(uint32_t threadId, int fd)
{
    mClientDatas[threadId].erase(fd);
}

void MultiThreadClientBase::ThreadFunc(uint32_t threadId)
{
    // 创建Epoll池
    CppEpollManager mEpollManager(mEpollSize);

    epoll_event ev;
    ev.events = EPOLLOUT | EPOLLRDHUP;      // 连接之后监听可写事件

    // 创建客户端并将其加入epoll池
    for (uint32_t i = 0; i < mClientCountPerThread; ++i)
    {
        ev.data.fd = ConnectServer();
        CHECK_CONTINUE(mpCppLog, ev.data.fd >= 0, ev.data.fd, CppLog::ERROR);
        mEpollManager.AddOrModFd(ev.data.fd, ev);
        mClientDatas[threadId][ev.data.fd] = MakeNewClientData();
        mClientDatas[threadId][ev.data.fd]->uniqueFd.Reset(ev.data.fd);
    }

    int waitTime = 0;
    function<int32_t(epoll_event &inevent)> readFunc = bind(&MultiThreadClientBase::ProcRead, this, threadId, placeholders::_1);
    function<int32_t(epoll_event &inevent)> writeFunc = bind(&MultiThreadClientBase::ProcWrite, this, threadId, placeholders::_1);
    function<void(int fd)> deleteFdFunc = bind(&MultiThreadClientBase::ProcDeleteFd, this, threadId, placeholders::_1);

    while (!gClientStop)
    {
        // 事件循环
        mEpollManager.Wait(readFunc, writeFunc, deleteFdFunc, waitTime);
    }

    DEBUG_ILOG(mpCppLog, "exit thread[%u].", threadId);
}

int32_t MultiThreadClientBase::ConnectServer()
{
    char buf[1024];
    int ret;

    // 解析域名，如果是IP则不用解析，如果出错，显示错误信息
    hostent nlp_host, *p_host;
    gethostbyname_r(mServerAddr.c_str(), &nlp_host, buf, sizeof(buf), &p_host, &ret);
    CHECK_RETURN_F(mpCppLog, ret == 0, -1, CppLog::ERROR, "gethostbyname失败,errno[%d],error[%s].", errno, strerror(errno));

    // 设置pin变量，包括协议、地址、端口等，此段可直接复制到自己的程序中
    sockaddr_in pin;
    bzero(&pin, sizeof(pin));
    pin.sin_family = AF_INET;                   // AF_INET表示使用IPv4
    pin.sin_addr.s_addr = ((struct in_addr *)(nlp_host.h_addr))->s_addr;       // TODO:这一句是干嘛用的？好像没什么用，去掉不影响。
    pin.sin_port = htons(mServerPort);

    // 建立socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_RETURN_F(mpCppLog, fd >= 0, fd, CppLog::ERROR, "socket失败,ret[%d],errno[%d],error[%s].", fd, errno, strerror(errno));

    // 中途出错，释放FD
    UniqueFd uniqFd(fd);

    // 建立连接
    ret = connect(fd, (struct sockaddr*)&pin, sizeof(pin));
    ERROR_RETURN_F(mpCppLog, ret, CppLog::ERROR, "connect失败,errno[%d],error[%s].", errno, strerror(errno));

    int flags = fcntl(fd, F_GETFL, 0);
    ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    ERROR_RETURN_F(mpCppLog, ret, CppLog::ERROR, "fcntl O_NONBLOCK失败,errno[%d],error[%s].", errno, strerror(errno));

    flags = 1;
    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
    ERROR_RETURN_F(mpCppLog, ret, CppLog::ERROR, "setsockopt TCP_NODELAY失败,errno[%d],error[%s].", errno, strerror(errno));

    // 初始化连接
    InitConnection(fd);

    return uniqFd.Release();
}

int32_t MultiThreadClientBase::Run()
{
    list<thread> threads;
    mClientDatas.resize(mClientThreadCount);
    for (uint32_t i = 0; i < mClientThreadCount; ++i)
    {
        threads.push_back(thread(&MultiThreadClientBase::ThreadFunc, this, i));
    }

    uint64_t lastSuccess = 0;
    uint64_t lastFail = 0;

    // 为了不被其他线程影响，这里需要将全局数据拷贝下来
    uint64_t currTotalSuccess = 0;
    uint64_t currTotalFail = 0;

    uint32_t second = 0;

    DEBUG_ILOG(mpCppLog, "开始压测[%s:%u],线程数[%u],每线程客户端数[%u],总客户端数[%u],压测[%u]秒.",
               mServerAddr.c_str(), mServerPort, mClientThreadCount, mClientCountPerThread,
               mClientThreadCount * mClientCountPerThread, mRunSecond);
    while (true)
    {
        sleep(1);
        currTotalSuccess = gSuccessCount;
        currTotalFail = gFailCount;

        DEBUG_ILOG(mpCppLog, "周期成功[%llu],周期失败[%llu],周期成功率[%.2lf%%],最大耗时[%llu],最小耗时[%llu],平均耗时[%llu].",
                   currTotalSuccess - lastSuccess, currTotalFail - lastFail,
                   (currTotalSuccess - lastSuccess + currTotalFail - lastFail) == 0 ? 0 : (currTotalSuccess - lastSuccess) * 100.0 / (currTotalSuccess - lastSuccess + currTotalFail - lastFail),
                   gCurrMaxTimeUs, gCurrMinTimeUs,
                   (currTotalSuccess - lastSuccess + currTotalFail - lastFail) == 0 ? 0 : gCurrTotalTimeUs / (currTotalSuccess - lastSuccess + currTotalFail - lastFail));
        DEBUG_ILOG(mpCppLog, "总成功[%llu],总失败[%llu],总成功率[%.2lf%%],总最大耗时[%llu],总最小耗时[%llu],总平均耗时[%llu].",
                   currTotalSuccess, currTotalFail,
                   (currTotalSuccess + currTotalFail) == 0 ? 0 : currTotalSuccess * 100.0 / (currTotalSuccess + currTotalFail),
                   gMaxTimeUs, gMinTimeUs,
                   (currTotalSuccess + currTotalFail) == 0 ? 0 : gTotalTimeUs / (currTotalSuccess + currTotalFail));

        lastSuccess = currTotalSuccess;
        lastFail = currTotalFail;
        gCurrTotalTimeUs = 0;
        gCurrMinTimeUs = 0;
        gCurrMaxTimeUs = 0;

        if (++second >= mRunSecond)
        {
            gClientStop = true;
            break;
        }
    }

    DEBUG_ILOG(mpCppLog, "结束压测,线程数[%u],每线程客户端数[%u],总客户端数[%u],成功[%llu],失败[%llu],成功率[%.2lf%%],每秒请求[%llu],最大耗时[%llu],最小耗时[%llu],平均耗时[%llu].",
               mClientThreadCount, mClientCountPerThread, mClientThreadCount * mClientCountPerThread,
               currTotalSuccess, currTotalFail,
               (currTotalSuccess + currTotalFail) == 0 ? 0 : currTotalSuccess * 100.0 / (currTotalSuccess + currTotalFail),
               (currTotalSuccess + currTotalFail) / mRunSecond,
               gMaxTimeUs, gMinTimeUs,
               (currTotalSuccess + currTotalFail) == 0 ? 0 : gTotalTimeUs / (currTotalSuccess + currTotalFail));

    for (auto &t : threads)
    {
        t.join();
    }

    return 0;
}

#endif
