#ifndef __CYGWIN__
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

#include <map>
#include <list>
#include <mutex>
#include <thread>

#include <CppString.h>
#include <CppArray.h>
#include <CppTime.h>
#include <CppNet.h>

#include "global.h"

#include "gtest/gtest.h"

// zookeeper
#include <zookeeper/recordio.h>
#include <zookeeper/proto.h>
#include <zookeeper_src/zk_adaptor.h>
#include <zookeeper_generated/zookeeper.jute.h>

// TODO
// 疑问：
//  客户端开1个线程，服务端1个线程，可以跑到20W，但是客户端开2个线程，服务端1个线程，只有14W左右，为什么呢？是跟epoll_wait的时间有关吗？
// 待实现功能：
//  epoll_wait等待的时间动态调整
//  读缓存
//  写缓存
//  粘包检测
//  边缘触发
//  开始时输出信息

using namespace std;

// 服务端类型
enum ServerType
{
    SINGLE_THREAD_SERVER,       // 单线程服务端
    MULTI_THREAD_SERVER         // 多线程服务端
};

// 公共配置
static const string SERV_NAME = "127.0.0.1";
static const uint16_t SERV_PORT = 20001;

// 客户端配置
static const uint32_t TOTAL_SECOND = 15;                    // 总共执行的秒数
static const uint32_t CLIENT_COUNT_PER_THREAD = 30;         // 每个客户端线程包含的客户端数量
static const uint32_t CLIENT_THREAD_COUNT = 4;              // 客户端线程数

// 服务端配置
static const ServerType SERVER_TYPE = MULTI_THREAD_SERVER;  // 服务端类型
static const uint32_t SERVER_THREAD_COUNT = 4;              // 服务端线程数，仅在SERVER_TYPE=MULTI_THREAD_SERVER时有效

// 常量
static const uint32_t CLIENT_EPOLL_SIZE = 100;              // 客户端Epoll池容量
static const uint32_t BUF_SIZE = 256;                       // 读写数据缓冲区大小

static const uint32_t WAIT_TIME_MS = 10;                    // 服务端accept每次epoll_wait的时间(毫秒)
static const uint32_t SERVER_EPOLL_SIZE = 100;              // 服务端Epoll池容量

// 全局变量
map<int, uint64_t> gServerData;                             // <fd,服务端数据>
bool gServerStop = false;                                   // 服务端结束标志
bool gServerStart = false;                                  // 服务端开始标志

class ClientData :public PressCallClientDataBase
{
public:
    uint64_t value = 0;
};

class MultiThreadClient :public MultiThreadClientBase
{
public:
    MultiThreadClient(const std::string &serverAddr, uint16_t serverPort,
                      uint32_t runSecond, uint32_t clientThreadCount,
                      uint32_t clientCountPerThread,
                      uint32_t epollSize, CppLog *mpCppLog) :MultiThreadClientBase(
                          serverAddr, serverPort, runSecond, clientThreadCount, clientCountPerThread,
                          epollSize, mpCppLog)
    {
    }

protected:
    virtual bool CheckResponse(uint32_t threadId, int fd, const string &bufStr);
    virtual const string GetSendData(uint32_t threadId, int fd);
    virtual shared_ptr<PressCallClientDataBase> MakeNewClientData()
    {
        return make_shared<ClientData>();
    }
};

bool MultiThreadClient::CheckResponse(uint32_t threadId, int fd, const string &bufStr)
{
    // 比较回复，看是否与期望值相等，期望值是发送过去的数据+1
    uint64_t expectValue = static_cast<ClientData *>(GetClientData(threadId, fd).get())->value + 1;

    // 先保存成指针，再解引用，避免产生warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
    const uint64_t *pAactualValue = reinterpret_cast<const uint64_t *>(bufStr.c_str());
    uint64_t actualValue = *pAactualValue;
    actualValue = CppNet::Ntohll(actualValue);
    if (actualValue == expectValue)
    {
        // 成功
        return true;
    }
    else
    {
        //失败
        DEBUG_LOG("BAD:expect[%lu],actual[%lu].", expectValue, actualValue);
        return false;
    }
}

const string MultiThreadClient::GetSendData(uint32_t threadId, int fd)
{
    uint64_t data = CppTime::GetUTime();
    static_cast<ClientData *>(GetClientData(threadId, fd).get())->value = data;

    data = CppNet::Htonll(data);
    return string(reinterpret_cast<char *>(&data), sizeof(data));
}

namespace SingleThreadServer
{

/** 清除客户端FD并且删除数据
 *
 * @param   int fd
 * @retval  void
 * @author  moon
 */
void CloseAndClearFd(int fd)
{
    close(fd);
    gServerData.erase(fd);
}

/** 服务端处理读请求
 *
 * @param   epoll_event & inevent
 * @param   int epfd
 * @retval  int32_t
 * @author  moon
 */
int32_t ServerProcRead(epoll_event &inevent, int epfd)
{
    char buf[BUF_SIZE];

    // 读取请求
    int32_t ret = read(inevent.data.fd, buf, sizeof(buf));
    if (ret < 0)
    {
        DEBUG_LOG("read error,ret[%d],fd[%d],errno[%d],error[%s].", ret, inevent.data.fd, errno, strerror(errno));
        return ret;
    }

    if (ret == 0)
    {
        DEBUG_LOG("read=0.");
        return -1;
    }

    // 先保存成指针，再解引用，避免产生warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
    uint64_t *pValue = reinterpret_cast<uint64_t *>(buf);
    gServerData[inevent.data.fd] = *pValue;
    gServerData[inevent.data.fd] = CppNet::Htonll(gServerData[inevent.data.fd]);

    // 检查可写状态
    inevent.events = EPOLLOUT | EPOLLRDHUP;
    ret = epoll_ctl(epfd, EPOLL_CTL_MOD, inevent.data.fd, &inevent);
    if (ret < 0)
    {
        DEBUG_LOG("epoll_ctl failed,ret[%d],fd[%d],errno[%d],error[%s].", ret, inevent.data.fd, errno, strerror(errno));
        return ret;
    }

    return 0;
}

/** 服务端处理写请求
 *
 * @param   epoll_event & inevent
 * @param   int epfd
 * @retval  int32_t
 * @author  moon
 */
int32_t ServerProcWrite(epoll_event &inevent, int epfd)
{
    // 构造应答包
    uint64_t rspValue = gServerData[inevent.data.fd] + 1;
    rspValue = CppNet::Htonll(rspValue);

    // 回复应答包
    int32_t ret = write(inevent.data.fd, &rspValue, sizeof(rspValue));
    if (ret < 0)
    {
        DEBUG_LOG("write error,event[%u],ret[%d],errno[%d],error[%s].", inevent.events, ret, errno, strerror(errno));
        return ret;
    }

    // 检查可读状态
    inevent.events = EPOLLIN | EPOLLRDHUP;
    ret = epoll_ctl(epfd, EPOLL_CTL_MOD, inevent.data.fd, &inevent);
    if (ret < 0)
    {
        DEBUG_LOG("epoll_ctl failed,ret[%d],fd[%d],errno[%d],error[%s].", ret, inevent.data.fd, errno, strerror(errno));
        return ret;
    }

    return 0;
}

/** 创建服务器监听端口
 *
 * @retval  int
 * @author  moon
 */
int CreateServerSocket()
{
    int32_t ret = 0;

    // 创建监听Socket
    int serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverFd < 0)
    {
        DEBUG_LOG("Create socket failed.");
        return serverFd;
    }

    // 设置监听端口非阻塞
    int flags = fcntl(serverFd, F_GETFL, 0);
    ret = fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0)
    {
        DEBUG_LOG("fcntl failed.");
        close(serverFd);
        return ret;
    }

    // 设置REUSEADDR标识，服务器重启可以快速使用这个端口，避免在TIME_WAIT状态无法重新监听这个端口
    int rep = 1;
    ret = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &rep, sizeof(rep));
    if (ret < 0)
    {
        DEBUG_LOG("set reuse addr failed.");
        close(serverFd);
        return ret;
    }

    // 监听端口设置
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // 绑定Socket到端口
    ret = bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (ret < 0)
    {
        DEBUG_LOG("bind failed.");
        close(serverFd);
        return ret;
    }

    // 开始监听
    const uint32_t LISTEN_BACKLog = 10;
    ret = listen(serverFd, LISTEN_BACKLog);
    if (ret < 0)
    {
        DEBUG_LOG("listen failed.");
        close(serverFd);
        return ret;
    }

    DEBUG_LOG("Begin to listen port[%u]... ", SERV_PORT);
    gServerStart = true;

    return serverFd;
}

/** 创建服务端Epoll池
 *
 * @param   int serverFd
 * @retval  int
 * @author  moon
 */
int CreateEpollFd(int serverFd)
{
    int epollFd = epoll_create(SERVER_EPOLL_SIZE);
    if (epollFd < 0)
    {
        DEBUG_LOG("epoll create failed.");
        close(serverFd);
        return epollFd;
    }

    // 将监听的端口加入到Epoll池管理
    epoll_event ev;
    ev.data.fd = serverFd;
    ev.events = EPOLLIN | EPOLLRDHUP;
    int32_t ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev);
    if (ret < 0)
    {
        DEBUG_LOG("epoll ctl failed,fd[%d].", epollFd);
        close(serverFd);
        close(epollFd);
        return ret;
    }

    return epollFd;
}

/** 启动Server
 *
 * @retval  int32_t
 * @author  moon
 */
int32_t StartServer()
{
    // 忽略SIGPIPE信号，防止客户端关闭后，服务端往Socket中写入数据导致服务端收到此信号导致服务挂掉
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    int serverFd = CreateServerSocket();
    int epollFd = CreateEpollFd(serverFd);
    int32_t ret;

    epoll_event events[SERVER_EPOLL_SIZE];
    sockaddr_in cliAddr;
    socklen_t addrLen = sizeof(cliAddr);

    while (!gServerStop)
    {
        int nfds = epoll_wait(epollFd, events, ARRAY_SIZE(events), WAIT_TIME_MS);
        if (nfds < 0)
        {
            DEBUG_LOG("epoll_wait error,ret[%d],errno[%d],error[%s].", nfds, errno, strerror(errno));
            continue;
        }

        if (nfds == 0)   // 超时
        {
            // DEBUG_LOG("epoll wait timeout.");
            continue;
        }

        // 读取到客户端
        for (int32_t i = 0; i < nfds; ++i)
        {
            if (events[i].events & EPOLLIN)
            {
                if (events[i].data.fd == serverFd)
                {
                    // 新连接
                    int clientFd = accept(events[i].data.fd, (struct sockaddr*)&cliAddr, &addrLen);
                    if (clientFd < 0)
                    {
                        DEBUG_LOG("accept failed,fd[%d],ret[%d],errno[%d],error[%s].",
                                  events[i].data.fd, clientFd, errno, strerror(errno));
                        continue;
                    }

                    DEBUG_LOG("New client, fd[%d].");

                    // 设置数据链接非阻塞
                    int flags = fcntl(clientFd, F_GETFL, 0);
                    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

                    // 添加进epoll fd里
                    epoll_event ev;
                    ev.data.fd = clientFd;
                    ev.events = EPOLLIN;

                    ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);
                    if (ret < 0)
                    {
                        DEBUG_LOG("epoll_ctl failed,ret[%d],fd[%d],errno[%d],error[%s].",
                                  ret, clientFd, errno, strerror(errno));
                        CloseAndClearFd(clientFd);
                        continue;
                    }
                }
                else if (events[i].events & EPOLLRDHUP)
                {
                    // 对端断开，删除fd
                    ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    CloseAndClearFd(events[i].data.fd);
                    //DEBUG_LOG("Delete fd[%d] from epoll success.", events[i].data.fd);
                }
                else
                {
                    // 处理读请求
                    ret = ServerProcRead(events[i], epollFd);

                    // 失败删除fd
                    if (ret != 0)
                    {
                        ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        if (ret < 0)
                        {
                            DEBUG_LOG("epoll_ctl failed,ret[%d],fd[%d],errno[%d],error[%s].",
                                      ret, events[i].data.fd, errno, strerror(errno));
                            continue;
                        }

                        CloseAndClearFd(events[i].data.fd);
                        DEBUG_LOG("Delete fd[%d] from epoll success.", events[i].data.fd);
                    }
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                // 处理写请求
                ret = ServerProcWrite(events[i], epollFd);

                // 失败删除fd
                if (ret != 0)
                {
                    ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    if (ret < 0)
                    {
                        DEBUG_LOG("epoll_ctl failed,ret[%d],fd[%d],errno[%d],error[%s].",
                                  ret, events[i].data.fd, errno, strerror(errno));
                        continue;
                    }

                    CloseAndClearFd(events[i].data.fd);
                    DEBUG_LOG("Delete fd[%d] from epoll success.", events[i].data.fd);
                }
            }
            else
            {
                continue;
            }
        }
    }

    close(serverFd);
    close(epollFd);
    return 0;
}

}

// 服务端工作线程
class ServerWorkThead
{
public:
    ServerWorkThead(uint32_t threadId) : mEpollManager(100), mThreadId(threadId)
    {
    }

    virtual int32_t ProcWrite(epoll_event &inevent);
    virtual int32_t ProcRead(epoll_event &inevent);
    virtual void ProcDeleteFd(int fd);

    void Run();

    void AddClientFd(int clientFd);

private:
    CppEpollManager mEpollManager;
    uint32_t mThreadId;
    map<int, uint64_t> mClientDatas;                         // <客户端fd,数据>
    mutex mLock;
};

/** 服务端处理写请求
*
* @param   epoll_event & inevent
* @param   int epfd
* @retval  int32_t
* @author  moon
*/
int32_t ServerWorkThead::ProcWrite(epoll_event &inevent)
{
    // 构造应答包
    uint64_t rspValue = mClientDatas[inevent.data.fd] + 1;
    rspValue = CppNet::Htonll(rspValue);

    // 回复应答包
    int32_t ret = write(inevent.data.fd, &rspValue, sizeof(rspValue));
    if (ret < 0)
    {
        DEBUG_LOG("write error,event[%u],ret[%d],errno[%d],error[%s].", inevent.events, ret, errno, strerror(errno));
        return ret;
    }

    return 0;
}

/** 服务端处理读请求
*
* @param   epoll_event & inevent
* @retval  int32_t
* @author  moon
*/
int32_t ServerWorkThead::ProcRead(epoll_event &inevent)
{
    char buf[BUF_SIZE];

    // 读取请求
    int32_t ret = read(inevent.data.fd, buf, sizeof(buf));
    if (ret < 0)
    {
        DEBUG_LOG("read error,ret[%d],fd[%d],errno[%d],error[%s].", ret, inevent.data.fd, errno, strerror(errno));
        return ret;
    }

    if (ret == 0)
    {
        DEBUG_LOG("read=0.");
        return -1;
    }

    // 先保存成指针，再解引用，避免产生warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
    uint64_t *pValue = reinterpret_cast<uint64_t *>(buf);
    mClientDatas[inevent.data.fd] = *pValue;
    mClientDatas[inevent.data.fd] = CppNet::Ntohll(mClientDatas[inevent.data.fd]);

    return 0;
}

void ServerWorkThead::ProcDeleteFd(int fd)
{
    close(fd);
    mClientDatas.erase(fd);
}

void ServerWorkThead::AddClientFd(int clientFd)
{
    // 添加进epoll fd里
    epoll_event ev;
    ev.data.fd = clientFd;
    ev.events = EPOLLIN;

    mEpollManager.AddOrModFd(clientFd, ev);
}

void ServerWorkThead::Run()
{
    int waitTime = 0;
    function<int32_t(epoll_event &inevent)> readFunc = bind(&ServerWorkThead::ProcRead, this, placeholders::_1);
    function<int32_t(epoll_event &inevent)> writeFunc = bind(&ServerWorkThead::ProcWrite, this, placeholders::_1);
    function<void(int fd)> deleteFdFunc = bind(&ServerWorkThead::ProcDeleteFd, this, placeholders::_1);

    while (!gServerStop)
    {
        mEpollManager.Wait(readFunc, writeFunc, deleteFdFunc, waitTime);
    }

    DEBUG_LOG("exit thread[%u].", mThreadId);
}

// 多线程服务端
//  监听线程：1个，用于accept新的客户端连接，获得的连接平均分发到各个数据处理线程（TODO：这里可以使用多种分配模型，比如根据负载，根据客户端数等等）
//  数据处理线程：N个，N可配置，每个线程管理一个Epoll池
//
// TODO：2个模型，选择哪个呢？
//  1、工作线程线程处理连接，主线程只把accept到的fd传给Server线程
//  2、工作线程线程只处理数据，主线程来读取数据
//  先实现第一个看看吧
class MultiThreadServer
{
public:
    /** 启动Server
    *
    * @retval  int32_t
    * @author  moon
    */
    int32_t StartServer();

protected:

    /** 创建服务器监听端口
    *
    * @retval  int
    * @author  moon
    */
    int CreateServerSocket();

    /** 创建服务端Epoll池
    *
    * @param   int serverFd
    * @retval  int
    * @author  moon
    */
    int CreateEpollFd(int serverFd);
};

int MultiThreadServer::CreateServerSocket()
{
    // 创建监听Socket
    int serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    CHECK_RETURN_F(&cppLog, serverFd >= 0, serverFd, CppLog::ERROR, "socket失败,errno[%d],error[%s].", errno, strerror(errno));
    UniqueFd uniqServerFd(serverFd);

    // 设置监听端口非阻塞
    int flags = fcntl(serverFd, F_GETFL, 0);
    int32_t ret = fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "fcntl失败,errno[%d],error[%s].", errno, strerror(errno));

    // 设置REUSEADDR标识，服务器重启可以快速使用这个端口，避免在TIME_WAIT状态无法重新监听这个端口
    flags = 1;
    ret = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "setsockopt SO_REUSEADDR失败,errno[%d],error[%s].", errno, strerror(errno));

    flags = 1;
    ret = setsockopt(serverFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "setsockopt TCP_NODELAY失败,errno[%d],error[%s].", errno, strerror(errno));

    // 监听端口设置
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);
    servAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // 绑定Socket到端口
    ret = bind(serverFd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "bind失败,errno[%d],error[%s].", errno, strerror(errno));

    // 开始监听
    const uint32_t LISTEN_BACKLog = 10;
    ret = listen(serverFd, LISTEN_BACKLog);
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "listen失败,errno[%d],error[%s].", errno, strerror(errno));

    DEBUG_LOG("服务启动成功,监听端口[%u].", SERV_PORT);
    gServerStart = true;

    return uniqServerFd.Release();
}

int MultiThreadServer::CreateEpollFd(int serverFd)
{
    int epollFd = epoll_create(SERVER_EPOLL_SIZE);
    CHECK_RETURN_F(&cppLog, epollFd >= 0, epollFd, CppLog::ERROR, "epoll_create失败,errno[%d],error[%s].", errno, strerror(errno));
    UniqueFd uniqEpollFd(epollFd);

    // 将监听的端口加入到Epoll池管理
    epoll_event ev;
    ev.data.fd = serverFd;
    ev.events = EPOLLIN | EPOLLRDHUP;
    int32_t ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev);
    ERROR_RETURN_F(&cppLog, ret, CppLog::ERROR, "epoll_ctl失败,errno[%d],error[%s].", errno, strerror(errno));

    return uniqEpollFd.Release();
}

int32_t MultiThreadServer::StartServer()
{
    map<int, UniqueFd> clientFds;

    // 忽略SIGPIPE信号，防止客户端关闭后，服务端往Socket中写入数据导致服务端收到此信号导致服务挂掉
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    int serverFd = CreateServerSocket();
    CHECK_RETURN(&cppLog, serverFd >= 0, serverFd, CppLog::ERROR);
    UniqueFd uniqServerFd(serverFd);

    int epollFd = CreateEpollFd(serverFd);
    CHECK_RETURN(&cppLog, epollFd >= 0, epollFd, CppLog::ERROR);
    UniqueFd uniqEpollFd(epollFd);

    epoll_event events[SERVER_EPOLL_SIZE];
    sockaddr_in cliAddr;
    socklen_t addrLen = sizeof(cliAddr);

    // 创建工作线程
    list<thread> threads;
    vector<shared_ptr<ServerWorkThead>> workThreads;
    for (uint32_t i = 0; i < SERVER_THREAD_COUNT; ++i)
    {
        workThreads.push_back(make_shared<ServerWorkThead>(i));
        threads.push_back(thread(&ServerWorkThead::Run, workThreads[i].get()));
    }

    // 动态调整wait的时间，有数据的话，则缩短时间
    int waitTime = WAIT_TIME_MS;
    while (!gServerStop)
    {
        int fdsCount = epoll_wait(epollFd, events, ARRAY_SIZE(events), waitTime);
        waitTime = WAIT_TIME_MS;
        CHECK_CONTINUE_F(&cppLog, fdsCount >= 0, fdsCount, CppLog::ERROR, "epoll_wait错误,errno[%d],error[%s].",
                         errno, strerror(errno));

        if (fdsCount == 0)
        {
            // DEBUG_LOG("epoll wait timeout.");
            continue;
        }

        // 读取到客户端新连接，只有一个FD？
        // 修改下次wait的时间为0ms，避免有客户端的时候继续等待
        waitTime = 0;
        if (events[0].events & EPOLLIN)
        {
            int clientFd = accept(events[0].data.fd, (struct sockaddr*)&cliAddr, &addrLen);
            CHECK_CONTINUE_F(&cppLog, clientFd >= 0, fdsCount, CppLog::ERROR, "accept错误,errno[%d],error[%s].",
                             errno, strerror(errno));

            // 设置数据链接非阻塞，测试结果表示对程序的性能没有明显影响。
//          int flags = fcntl(clientFd, F_GETFL, 0);
//          fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
// 
//          flags = 1;
//          int ret = setsockopt(clientFd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(flags));
//          if (ret != 0)
//          {
//              DEBUG_LOG("set TCP_NODELAY failed.");
//              close(clientFd);
//              continue;
//          }

            static uint32_t clientId = 0;
            workThreads[clientId % SERVER_THREAD_COUNT]->AddClientFd(clientFd);
            clientFds[clientFd] = UniqueFd(clientFd);
            DEBUG_LOG("New client[%u], fd[%d], Server thread[%d].",
                      clientId, clientFd, clientId % SERVER_THREAD_COUNT);
            ++clientId;
        }
    }

    for (auto &th : threads)
    {
        th.join();
    }

    return 0;
}

TEST(CppNet, DISABLED_EpollTest)
{
    gServerStop = false;
    MultiThreadServer multiServer;
    shared_ptr<thread> pServer;

    // 创建一个服务端线程
    if (SERVER_TYPE == MULTI_THREAD_SERVER)
    {
        pServer = make_shared<thread>(&MultiThreadServer::StartServer, &multiServer);
    }
    else
    {
        pServer = make_shared<thread>(SingleThreadServer::StartServer);
    }

    // 忙等待服务端开启，TODO：可使用条件变量代替
    while (!gServerStart)
    {
        usleep(10);
    }

    // 执行客户端线程，此处阻塞知道客户端全部退出
    MultiThreadClient client(SERV_NAME, SERV_PORT, TOTAL_SECOND, CLIENT_THREAD_COUNT,
                             CLIENT_COUNT_PER_THREAD, CLIENT_EPOLL_SIZE, &cppLog);
    client.Run();

    // 通知服务端停止
    gServerStop = true;

    // 等待线程返回
    pServer->join();

    DEBUG_LOG("Finished,totalSuccess[%llu],totalFail[%llu],successPercent[%.3f%].",
              client.gSuccessCount, client.gFailCount,
              (double)(client.gSuccessCount) / (client.gSuccessCount + client.gFailCount) * 100);
}

/*
 * Redis压测工具，性能比redis-benchmark稍高点
 * 性能数据：
 *  本机测试（办公机虚拟机），redis-benchmark为22W，这个压测可达26W（非pipeline，纯PING）
 **/
class PressCallRedisClient :public MultiThreadClientBase
{
public:
    PressCallRedisClient(const std::string &serverAddr, uint16_t serverPort,
                         uint32_t runSecond, uint32_t clientThreadCount,
                         uint32_t clientCountPerThread,
                         uint32_t epollSize, CppLog *mpCppLog) :MultiThreadClientBase(
                             serverAddr, serverPort, runSecond, clientThreadCount, clientCountPerThread,
                             epollSize, mpCppLog)
    {
    }

protected:
    virtual bool CheckResponse(uint32_t threadId, int fd, const string &bufStr);
    virtual const string GetSendData(uint32_t threadId, int fd);
    virtual shared_ptr<PressCallClientDataBase> MakeNewClientData()
    {
        return make_shared<PressCallClientDataBase>();
    }
};

bool PressCallRedisClient::CheckResponse(uint32_t threadId, int fd, const string &bufStr)
{
    static_cast<void>(threadId);
    static_cast<void>(fd);
    if (!bufStr.empty() && *bufStr.begin() == '+')
    {
        return true;
    }

    return false;
}

const string PressCallRedisClient::GetSendData(uint32_t threadId, int fd)
{
    static_cast<void>(threadId);
    static_cast<void>(fd);
    return "PING\r\n";
}

TEST(CppNet, DISABLED_PressCallRedis)
{
    PressCallRedisClient presscallRedisClient("127.0.0.1", 6379, TOTAL_SECOND, CLIENT_THREAD_COUNT,
                                              CLIENT_COUNT_PER_THREAD, CLIENT_EPOLL_SIZE, &cppLog);
    presscallRedisClient.Run();
}

/*
 * Zookeeper压测
 * ping：结束压测,线程数[2],每线程客户端数[15],总客户端数[30],成功[3552420],失败[0],成功率[100.00%],每秒请求[236828],最大耗时[59587],最小耗时[3],平均耗时[119].
 * get： 结束压测,线程数[2],每线程客户端数[15],总客户端数[30],成功[2659761],失败[0],成功率[100.00%],每秒请求[177317],最大耗时[13148],最小耗时[4],平均耗时[162].
 * set（"/a"->3字节value）： 结束压测,线程数[2],每线程客户端数[15],总客户端数[30],成功[574405],失败[0],成功率[100.00%],每秒请求[38293],最大耗时[66778],最小耗时[151],平均耗时[773].
 *
 * 3机器集群，压测follow：
 * set（"/a"->3字节value）：结束压测,线程数[2],每线程客户端数[15],总客户端数[30],成功[282823],失败[0],成功率[100.00%],每秒请求[18854],最大耗时[99750],最小耗时[382],平均耗时[1571].
 *
 * 3机器集群，压测leader：
 * set（"/a"->3字节value）：结束压测,线程数[2],每线程客户端数[15],总客户端数[30],成功[364648],失败[0],成功率[100.00%],每秒请求[24309],最大耗时[101926],最小耗时[274],平均耗时[1212].
 *
 **/
class PressCallZookeeperClient :public MultiThreadClientBase
{
public:
    PressCallZookeeperClient(const std::string &serverAddr, uint16_t serverPort,
                             uint32_t runSecond, uint32_t clientThreadCount,
                             uint32_t clientCountPerThread,
                             uint32_t epollSize, CppLog *mpCppLog) :MultiThreadClientBase(
                                 serverAddr, serverPort, runSecond, clientThreadCount, clientCountPerThread,
                                 epollSize, mpCppLog)
    {
    }

protected:
    virtual bool CheckResponse(uint32_t threadId, int fd, const string &bufStr);
    virtual const string GetSendData(uint32_t threadId, int fd);
    virtual void InitConnection(int fd);
    virtual shared_ptr<PressCallClientDataBase> MakeNewClientData()
    {
        return make_shared<PressCallClientDataBase>();
    }
};

bool PressCallZookeeperClient::CheckResponse(uint32_t threadId, int fd, const string &bufStr)
{
    static_cast<void>(threadId);
    static_cast<void>(fd);
    struct ReplyHeader hdr;
    struct iarchive *ia = create_buffer_iarchive((char *)bufStr.c_str() + 4,
                                                 bufStr.size() - 4);
    int rc = deserialize_ReplyHeader(ia, "hdr", &hdr);
    close_buffer_iarchive(&ia);
    if (rc != 0)
    {
        ERROR_LOG("rc[%d].", rc);
        return false;
    }

    if (hdr.err != 0)
    {
        ERROR_LOG("hdr.err[%d].", hdr.err);
        return false;
    }

    return true;
}

int serialize_prime_connect(struct connect_req *req, char* buffer)
{
    //this should be the order of serialization
    int offset = 0;
    req->protocolVersion = htonl(req->protocolVersion);
    memcpy(buffer + offset, &req->protocolVersion, sizeof(req->protocolVersion));
    offset = offset + sizeof(req->protocolVersion);

    req->lastZxidSeen = zoo_htonll(req->lastZxidSeen);
    memcpy(buffer + offset, &req->lastZxidSeen, sizeof(req->lastZxidSeen));
    offset = offset + sizeof(req->lastZxidSeen);

    req->timeOut = htonl(req->timeOut);
    memcpy(buffer + offset, &req->timeOut, sizeof(req->timeOut));
    offset = offset + sizeof(req->timeOut);

    req->sessionId = zoo_htonll(req->sessionId);
    memcpy(buffer + offset, &req->sessionId, sizeof(req->sessionId));
    offset = offset + sizeof(req->sessionId);

    req->passwd_len = htonl(req->passwd_len);
    memcpy(buffer + offset, &req->passwd_len, sizeof(req->passwd_len));
    offset = offset + sizeof(req->passwd_len);

    memcpy(buffer + offset, req->passwd, sizeof(req->passwd));

    return 0;
}

const string PressCallZookeeperClient::GetSendData(uint32_t threadId, int fd)
{
    static_cast<void>(threadId);
    static_cast<void>(fd);
    int rc;
    oarchive *oa = create_buffer_oarchive();

    RequestHeader h = {get_xid(), ZOO_SETDATA_OP};
    struct SetDataRequest req;
    static const string setData = "123";
    req.path = "/a";
    req.data.buff = (char*)setData.c_str();
    req.data.len = setData.size();
    req.version = -1;
    rc = serialize_RequestHeader(oa, "header", &h);
    rc = serialize_SetDataRequest(oa, "req", &req);

//     RequestHeader h = {get_xid(), ZOO_GETDATA_OP};
//     GetDataRequest req = {(char*)"/a", false};
//     rc = serialize_RequestHeader(oa, "header", &h);
//     rc = serialize_GetDataRequest(oa, "req", &req);

//     RequestHeader h = {PING_XID, ZOO_PING_OP};
//     rc = serialize_RequestHeader(oa, "header", &h);

    string buf(get_buffer(oa), get_buffer_len(oa));
    close_buffer_oarchive(&oa, 0);

    static_cast<void>(rc);

    // 附加长度到前面（网络序）
    uint32_t hlen = htonl(buf.size());
    return string((char *)&hlen, sizeof(hlen)) + buf;
}

void PressCallZookeeperClient::InitConnection(int fd)
{
    // 发个握手包
    int rc;
    char buffer_req[HANDSHAKE_REQ_SIZE];
    int len = sizeof(buffer_req);
    int hlen = 0;
    struct connect_req req;
    req.protocolVersion = 0;
    req.sessionId = 0;
    req.passwd_len = sizeof(req.passwd);
    req.passwd[0] = '\0';
    req.timeOut = 0;
    req.lastZxidSeen = 0;
    hlen = htonl(len);
    /* We are running fast and loose here, but this string should fit in the initial buffer! */
    rc = send(fd, &hlen, sizeof(len), 0);
    serialize_prime_connect(&req, buffer_req);
    rc = rc < 0 ? rc : send(fd, buffer_req, len, 0);

    // 获得40个字节的回包
    uint32_t bytes = 0;
    while (bytes < 40)
    {
        rc = read(fd, buffer_req, sizeof(buffer_req));
        if (rc > 0)
        {
            bytes += rc;
        }
        usleep(1000);
    }
}

TEST(CppNet, DISABLED_PressCallZookeeper)
{
    PressCallZookeeperClient presscallZookeeperClient("127.0.0.1", 12187, TOTAL_SECOND, CLIENT_THREAD_COUNT,
                                                      CLIENT_COUNT_PER_THREAD, CLIENT_EPOLL_SIZE, &cppLog);
    presscallZookeeperClient.Run();
}

/*
* lester测压
**/
class PressCallLesterClient :public MultiThreadClientBase
{
public:
    PressCallLesterClient(const std::string &serverAddr, uint16_t serverPort,
                          uint32_t runSecond, uint32_t clientThreadCount,
                          uint32_t clientCountPerThread,
                          uint32_t epollSize, CppLog *mpCppLog) :MultiThreadClientBase(
                              serverAddr, serverPort, runSecond, clientThreadCount, clientCountPerThread,
                              epollSize, mpCppLog)
    {
    }

protected:
    virtual bool CheckResponse(uint32_t threadId, int fd, const string &bufStr);
    virtual const string GetSendData(uint32_t threadId, int fd);
    virtual shared_ptr<PressCallClientDataBase> MakeNewClientData()
    {
        return make_shared<PressCallClientDataBase>();
    }
};

bool PressCallLesterClient::CheckResponse(uint32_t threadId, int fd, const string &bufStr)
{
    static_cast<void>(threadId);
    static_cast<void>(fd);

    return bufStr == "pong";
}

const string PressCallLesterClient::GetSendData(uint32_t threadId, int fd)
{
    static_cast<void>(threadId);
    static_cast<void>(fd);
    return "ping";
}

TEST(CppNet, DISABLED_PressCallLester)
{
    PressCallLesterClient presscallClient("127.0.0.1", 8456, TOTAL_SECOND, CLIENT_THREAD_COUNT,
                                          CLIENT_COUNT_PER_THREAD, CLIENT_EPOLL_SIZE, &cppLog);
    presscallClient.Run();
}

#endif
