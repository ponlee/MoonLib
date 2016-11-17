#ifndef _CPP_NET_H_
#define _CPP_NET_H_

#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>

#ifndef USE_CPP_LOG_MACRO
#define USE_CPP_LOG_MACRO
#endif
#include "CppLog.h"
#include "CppString.h"

/* IP:端口对 */
class IpPort
{
public:
    std::string IP;
    uint16_t Port;

    IpPort(const std::string &ip = "", uint32_t port = 0) : IP(ip), Port(port)
    {
    };

    std::string ToString() const
    {
        return IP + ":" + CppString::ToString(Port);
    }

    bool operator==(const IpPort &right) const
    {
        return (IP == right.IP) && (Port == right.Port);
    }

    bool operator<(const IpPort &right) const
    {
        if (IP < right.IP)
        {
            return true;
        }
        else if (IP == right.IP)
        {
            return Port < right.Port;
        }
        else
        {
            return false;
        }
    }
};

class CppNet
{
public:
    static const uint32_t MAX_IP_STR_LEN;

    //************************************
    // Describe:  网络序的二进制IP转换成字符串形式的IP
    // Parameter: uint32_t ip
    // Returns:   std::std::string   IP字符串：8.8.8.8
    // Author:    moon
    //************************************
    static std::string NetIpToStr(uint32_t ip);

    //************************************
    // Describe:  字符串形式的IP转换成网络序的IP
    // Parameter: const std::string & ipStr      IP字符串：8.8.8.8
    // Returns:   std::uint32_t
    // Author:    moon
    //************************************
    static uint32_t StrToNetIp(const std::string &ipStr);

    /** 判断主机序是否是大端
     *
     * @retval  bool
     * @author  moontan
     */
    static bool IsBigendian()
    {
        uint32_t a = 1;
        // 大端在内存中内存排列为00 00 00 01
        return (reinterpret_cast<char *>(&a))[3] == 1;
    }

    /** 64位数据主机序转网络序
     *
     * @param   uint64_t ipHost
     * @retval  uint64_t
     * @author  moontan
     */
    static inline uint64_t Htonll(uint64_t dataHost)
    {
        // 网络序是大端，如果主机序也为大端，则直接返回即可
        if (IsBigendian())
        {
            return dataHost;
        }

        // 否则全部反过来
        uint32_t high = (dataHost >> 32) & 0xFFFFFFFF;
        uint32_t low = dataHost & 0xFFFFFFFF;
        return (static_cast<uint64_t>(htonl(low)) << 32) + htonl(high);
    }

    /** 64位数据网络序转主机序
     *
     * @param   uint64_t dataNet
     * @retval  uint64_t
     * @author  moontan
     */
    static inline uint64_t Ntohll(uint64_t dataNet)
    {
        return Htonll(dataNet);
    }

    //static bool NetIsOK(vector<IpPort> &ipPort);
};

// socket管理，自动释放
// 来自http://stackoverflow.com/questions/29614775/smart-pointer-to-manage-socket-file-descriptor
class UniqueFd
{
public:
    constexpr UniqueFd() noexcept = default;
    explicit UniqueFd(int fd) noexcept : mFd(fd)
    {
    }

    UniqueFd(UniqueFd &&u) noexcept : mFd(u.mFd)
    {
        u.mFd = -1;
    }

    ~UniqueFd()
    {
        if (-1 != mFd)
        {
            ::close(mFd);
        }
    }

    UniqueFd& operator=(UniqueFd&& u) noexcept
    {
        Reset(u.Release());
        return *this;
    }

    int Get() const noexcept
    {
        return mFd;
    }

    operator int() const noexcept
    {
        return mFd;
    }

    int Release() noexcept
    {
        int fd = mFd;
        mFd = -1;
        return fd;
    }

    void Reset(int fd = -1) noexcept
    {
        UniqueFd(fd).Swap(*this);
    }

    void Swap(UniqueFd& u) noexcept
    {
        std::swap(mFd, u.mFd);
    }

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    // in the global namespace to override ::close(int)
    friend int close(UniqueFd& u) noexcept
    {
        int closed = ::close(u.mFd);
        u.mFd = -1;
        return closed;
    }

private:
    int mFd = -1;
};

#ifndef __CYGWIN__
class epoll_event;

// 压测工具
// 每个连接对应的数据基类，需要时可以继承此类加上其他用户数据
class PressCallClientDataBase
{
public:
    PressCallClientDataBase() :uniqueFd()
    {
    }

    UniqueFd uniqueFd;
    timeval sendTime;       // 发送时间
};

class MultiThreadClientBase
{
public:

    MultiThreadClientBase(const std::string &serverAddr, uint16_t serverPort, uint32_t runSecond = 10,
                          uint32_t clientThreadCount = 1, uint32_t clientCountPerThread = 1,
                          uint32_t epollSize = 100, CppLog *pCppLog = NULL);

    /** 启动客户端线程
    *
    * @retval  int32_t
    * @author  moon
    */
    int32_t Run();

    /* 统计信息 */
    uint64_t gSuccessCount;                                     // 总成功数量
    uint64_t gFailCount;                                        // 总失败数量
    uint64_t gTotalTimeUs;                                      // 总耗时
    uint64_t gMinTimeUs;                                        // 最小耗时
    uint64_t gMaxTimeUs;                                        // 最大耗时
    uint64_t gCurrTotalTimeUs;                                  // 周期总耗时
    uint64_t gCurrMinTimeUs;                                    // 周期最小耗时
    uint64_t gCurrMaxTimeUs;                                    // 周期最大耗时

    bool gClientStop;

    string mServerAddr;                                         // 服务端地址
    uint16_t mServerPort;                                       // 服务端端口
    uint32_t mRunSecond;                                        // 压测时间
    uint32_t mClientThreadCount;                                // 客户端线程数
    uint32_t mClientCountPerThread;                             // 每个客户端线程包含的客户端数量
    uint32_t mEpollSize;                                        // 每个线程的Epoll池容量
    CppLog *mpCppLog;

protected:

    /** 初始化连接
     *
     * @param 	int fd
     * @retval 	void
     * @author 	moontan
     */
    virtual void InitConnection(int fd)
    {
        static_cast<void>(fd);
    }

    /** 获得要发送的数据
    *
    * @param 	int fd
    * @retval 	const string
    * @author 	moontan
    */
    virtual const string GetSendData(uint32_t threadId, int fd) = 0;

    /** 创建一个客户端数据，不需要填充数值
    *
    * @retval 	std::shared_ptr<PressCallClientDataBase>
    * @author 	moontan
    */
    virtual shared_ptr<PressCallClientDataBase> MakeNewClientData() = 0;

    /** 检查结果并做统计工作
     *
     * @param 	uint32_t threadId
     * @param 	int fd
     * @param 	const string & bufStr
     * @retval 	bool                        成功返回true，失败返回false
     * @author 	moontan
     */
    virtual bool CheckResponse(uint32_t threadId, int fd, const string &bufStr) = 0;

    int32_t ProcWrite(uint32_t threadId, epoll_event &inevent);

    int32_t ProcRead(uint32_t threadId, epoll_event &inevent);

    void ProcDeleteFd(uint32_t threadId, int fd);

    /** 连接服务器，获得fd
    *
    * @retval  int32_t         成功则>=0，否则失败
    * @author  moontan
    */
    int32_t ConnectServer();

    /** 客户端线程函数，每个线程管理一个epoll池，每个epoll池管理多个客户端
    *
    * @param   uint32_t threadId
    * @retval  void
    * @author  moon
    */
    void ThreadFunc(uint32_t threadId);

    shared_ptr<PressCallClientDataBase> GetClientData(uint32_t threadId, int fd)
    {
        return mClientDatas[threadId][fd];
    }

    vector<std::unordered_map<int, shared_ptr<PressCallClientDataBase>>> mClientDatas; // 线程ID->map<fd,用户数据>
};

class CppEpollManager
{
public:
    CppEpollManager(uint32_t size) throw(CppException);
    ~CppEpollManager();

    void AddOrModFd(int fd, epoll_event &event) throw(CppException);

    void DelFd(int fd) throw(CppException);

    void Wait(function<int32_t(epoll_event &inevent)> &readFunc,
              function<int32_t(epoll_event &inevent)> &writeFunc,
              function<void(int fd)> &deleteFdFunc,
              uint32_t timeOutMs) throw(CppException);

    /** 调用epoll_wait
     *
     * @param   epoll_event events[]
     * @param   uint32_t eventSize
     * @param   uint32_t timeOutMs
     * @retval  int32_t                 返回触发的fd数量
     * @author  moontan
     */
    int32_t Wait(epoll_event events[], uint32_t eventSize, uint32_t timeOutMs);

    virtual int GetFdFromEvent(epoll_event &event);

    virtual void SetFdToEvent(int fd, epoll_event &event);

    int32_t GetEpollFd()
    {
        return mEpollFd;
    }

    /** 为mEvents缩容
     *  mEvents仅会自动扩容，如果需要缩容，需要手工调用此函数
     *
     * @retval  void
     * @author  moontan
     */
    void ReleaseEventsMemory();

protected:
    virtual int32_t ProcWrite(epoll_event &inevent, function<int32_t(epoll_event &inevent)> &writeFunc) throw(CppException);

    virtual int32_t ProcRead(epoll_event &inevent, function<int32_t(epoll_event &inevent)> &readFunc) throw(CppException);

    int mEpollFd;

    std::set<int> mFds;                         // 保存mEpollFd中管理的所有的fd
    std::mutex mFdLock;                         // 用于mFds的线程保护，任何对mFds的操作都要加这个锁，因为其他线程有可能往本线程的实例中添加Fd
    std::shared_ptr<UniqueFd> mUniqEpollFd;
    std::vector<epoll_event> mEvents;           // 用于wait的event
};

#endif
#endif
