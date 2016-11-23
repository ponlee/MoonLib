#ifndef __CYGWIN__
#include <sys/epoll.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <iostream>

#include "gtest/gtest.h"

#include <string>
#include <list>
#include <thread>
#include <functional>

#include <zookeeper.h>
#include <zk_adaptor.h>

#include <CppZookeeper.h>

#include <CppLog.h>
#include <CppNet.h>
#include <CppArray.h>
#include <CppString.h>
#include <CppSystem.h>

#include "global.h"

using namespace std;
using namespace zookeeper;

static const string ZK_HOST = "127.0.0.1:2181";
static const string ZK_CONFIG_FILE_PATH = "data/zk_config.xml";
static const string TEST_ROOT_PATH = "/zk_test";
static const uint32_t MAX_DATA_LEN = 1024;
static const uint32_t MAX_PATH_LEN = 256;

string GetEventTypeStr(int type)
{
    if (type == ZOO_SESSION_EVENT)
    {
        return "ZOO_SESSION_EVENT";
    }

    if (type == ZOO_CREATED_EVENT)
    {
        return "ZOO_CREATED_EVENT";
    }

    if (type == ZOO_DELETED_EVENT)
    {
        return "ZOO_DELETED_EVENT";
    }

    if (type == ZOO_CHANGED_EVENT)
    {
        return "ZOO_CHANGED_EVENT";
    }

    if (type == ZOO_CHILD_EVENT)
    {
        return "ZOO_CHILD_EVENT";
    }
    if (type == ZOO_NOTWATCHING_EVENT)
    {
        return "ZOO_NOTWATCHING_EVENT";
    }

    return "invalid type";
}

void DisableZkLink()
{
    (void)CppSystem::ExcuteCommand("iptables -I INPUT -i lo -p tcp --dport 2181 -j DROP;iptables  -I OUTPUT -o lo -p tcp --sport 2181 -j DROP");
}

void EnableZkLink()
{
    (void)CppSystem::ExcuteCommand("iptables -I INPUT -i lo -p tcp --dport 2181 -j ACCEPT;iptables  -I OUTPUT -o lo -p tcp --sport 2181 -j ACCEPT");
}

#define ASYNC_BEGIN done = false
#define NOTIFY_SYNC done = true;sync_cond.notify_all()
#define WATI_SYNC sync_lock_u.lock();\
    while (!done)\
    {\
        sync_cond.wait(sync_lock_u);\
    }\
    sync_lock_u.unlock()

// 原始API测试
void ZookeeperApiTestGlobalWatcher1(zhandle_t *zh, int type, int state,
                                    const char *abs_path, void *p_zookeeper_context)
{
    INFOR_LOG("Watcher触发1,type[%d],path[%s],context_addr[%p].", type, abs_path, p_zookeeper_context);
}

void ZookeeperApiTestGlobalWatcher2(zhandle_t *zh, int type, int state,
                                    const char *abs_path, void *p_zookeeper_context)
{
    INFOR_LOG("Watcher触发2,type[%d],path[%s],context_addr[%p].", type, abs_path, p_zookeeper_context);
}

TEST(ZooKeeper, DISABLED_ZookeeperApiTest)
{
    EnableZkLink();
    zhandle_t *pZk = zookeeper_init(ZK_HOST.c_str(), &ZookeeperApiTestGlobalWatcher1, 30000, NULL, NULL, 0);
    ASSERT_TRUE(pZk != NULL);

    string TEST_NODE = "/zk_api_test";

    int ret = zoo_delete(pZk, TEST_NODE.c_str(), -1);

    // 目的：测试相同的context注册自定义Watcher会触发几次
    // 结果：相同的context只触发一次，但是不同的context可以触发多次
    //       因此C API内部是以Path-Context作为Key来管理自定义Watcher的
    INFOR_LOG("注册Watcher");
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher1, NULL, NULL); // 触发
    EXPECT_EQ(ZNONODE, ret);

    // 重复注册1
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher1, NULL, NULL);     // 不触发
    EXPECT_EQ(ZNONODE, ret);

    // 不同的context
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher1, &ret, NULL);     // 触发
    EXPECT_EQ(ZNONODE, ret);

    // 不同的回调函数
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher2, NULL, NULL);     // 触发
    EXPECT_EQ(ZNONODE, ret);

    // 不同的回调函数+不同的参数
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher2, &ret, NULL);     // 触发
    EXPECT_EQ(ZNONODE, ret);

    // 重复注册2
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher2, &ret, NULL);     // 不触发
    EXPECT_EQ(ZNONODE, ret);

    INFOR_LOG("创建节点,触发Watcher.");
    ret = zoo_create(pZk, TEST_NODE.c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
    EXPECT_EQ(ZOK, ret);

    // 测试删除节点的触发情况，使用3种方式注册Watcher，在删除节点的时候，只会触发一次
    ret = zoo_wexists(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher1, NULL, NULL);               // 触发
    EXPECT_EQ(ZOK, ret);

    char data;
    int buflen = 1;
    ret = zoo_wget(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher1, NULL, &data, &buflen, NULL);  // 不触发
    EXPECT_EQ(ZOK, ret);

    ScopedStringVector children;
    ret = zoo_wget_children(pZk, TEST_NODE.c_str(), &ZookeeperApiTestGlobalWatcher1, NULL, &children);    // 不触发
    EXPECT_EQ(ZOK, ret);

    INFOR_LOG("删除节点.");
    ret = zoo_delete(pZk, TEST_NODE.c_str(), -1);
    EXPECT_EQ(ZOK, ret);

    // 关闭连接
    zookeeper_close(pZk);
}

//#if 0
// ZookeeperManager同步API测试
TEST(ZooKeeper, DISABLED_ZkManagerSyncTest)
{
    ZookeeperManager zk_manager;
    zk_manager.Init(ZK_HOST, TEST_ROOT_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    const uint32_t COUNT = 3;
    INFOR_LOG("在根节点下创建[%u]个子节点，名字和值为数字.", COUNT);
    string real_path;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string node_name = CppString::ToString(i);
        ASSERT_EQ(ZOK, zk_manager.Create(node_name, node_name, &real_path));
    }

    string path_to_delete = CppString::ToString(COUNT - 2);
    INFOR_LOG("删除节点[%s].", path_to_delete.c_str());
    ASSERT_EQ(ZOK, zk_manager.Delete(path_to_delete, -1));
    INFOR_LOG("节点[%s]不存在了.", path_to_delete.c_str());
    ASSERT_EQ(ZNONODE, zk_manager.Exists(path_to_delete, NULL));

    INFOR_LOG("获得根目录下所有节点，还有[%u]个.", COUNT - 1);
    ScopedStringVector children;
    Stat stat;
    ASSERT_EQ(ZOK, zk_manager.GetChildren(TEST_ROOT_PATH, children, shared_ptr<WatcherFunType>(), &stat));
    ASSERT_EQ(static_cast<int32_t>(COUNT - 1), children.count);
    ASSERT_EQ(0, stat.version);

    Stat node_stat;
    INFOR_LOG("获得根目录下所有节点带stat，使用相对路径，还有[%u]个.", COUNT - 1);
    ASSERT_EQ(ZOK, zk_manager.GetChildren("", children, 0, &node_stat));
    ASSERT_EQ(static_cast<int32_t>(COUNT - 1), children.count);
    ASSERT_EQ(0, node_stat.version);

    INFOR_LOG("使用原有的ScopedStringVector对象再调用一次，不能有内存泄露.");
    ASSERT_EQ(ZOK, zk_manager.GetChildren(TEST_ROOT_PATH, children));
    ASSERT_EQ(static_cast<int32_t>(COUNT - 1), children.count);

    string path_to_op = CppString::ToString(COUNT - 1);
    INFOR_LOG("节点[%s]存在.", path_to_op.c_str());
    ASSERT_EQ(ZOK, zk_manager.Exists(path_to_op, &node_stat));

    INFOR_LOG("获得节点[%s]数据.", path_to_op.c_str());
    string node_data(MAX_DATA_LEN, '\0');
    int data_len = node_data.size() - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(path_to_op, const_cast<char *>(node_data.c_str()), &data_len, &node_stat));
    node_data[data_len] = '\0';
    ASSERT_EQ(1, data_len);
    ASSERT_EQ(0, strcmp(path_to_op.c_str(), node_data.c_str()));
    ASSERT_EQ(0, node_stat.version);

    string new_data = "new_data";
    INFOR_LOG("设置节点[%s]数据为[%s].", path_to_op.c_str(), new_data.c_str());
    ASSERT_EQ(ZOK, zk_manager.Set(path_to_op, new_data.c_str(), node_stat.version, &node_stat));
    ASSERT_EQ(1, node_stat.version);

    INFOR_LOG("再次获得节点[%s]数据.", path_to_op.c_str());
    data_len = node_data.size() - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(path_to_op, const_cast<char *>(node_data.c_str()), &data_len, NULL));
    node_data[data_len] = '\0';
    ASSERT_EQ(static_cast<int32_t>(new_data.size()), data_len);
    ASSERT_EQ(0, strcmp(new_data.c_str(), node_data.c_str()));

    INFOR_LOG("设置回节点[%s]数据为[%s].", path_to_op.c_str(), path_to_op.c_str());
    ASSERT_EQ(ZOK, zk_manager.Set(path_to_op, path_to_op.c_str(), node_stat.version, &node_stat));
    ASSERT_EQ(2, node_stat.version);

    INFOR_LOG("获得节点[%s]ACL数据.", path_to_op.c_str());
    ScopedAclVector acl;
    ASSERT_EQ(ZOK, zk_manager.GetAcl(path_to_op, acl, &node_stat));
    ASSERT_EQ(2, node_stat.version);
    ASSERT_EQ(0, node_stat.aversion);
    ASSERT_EQ(ZOO_OPEN_ACL_UNSAFE.count, acl.count);
    ASSERT_EQ(ZOO_OPEN_ACL_UNSAFE.data[0].perms, acl.data[0].perms);
    ASSERT_EQ(0, strcmp(ZOO_OPEN_ACL_UNSAFE.data[0].id.id, acl.data[0].id.id));
    ASSERT_EQ(0, strcmp(ZOO_OPEN_ACL_UNSAFE.data[0].id.scheme, acl.data[0].id.scheme));

    INFOR_LOG("设置节点[%s]ACL数据.", path_to_op.c_str());
    ASSERT_EQ(ZOK, zk_manager.SetAcl(path_to_op, node_stat.aversion, &ZOO_READ_ACL_UNSAFE));

    INFOR_LOG("再次获得节点[%s]ACL数据.", path_to_op.c_str());
    ASSERT_EQ(ZOK, zk_manager.GetAcl(path_to_op, acl, &node_stat));
    ASSERT_EQ(1, node_stat.aversion);
    ASSERT_EQ(ZOO_READ_ACL_UNSAFE.count, acl.count);
    ASSERT_EQ(ZOO_READ_ACL_UNSAFE.data[0].perms, acl.data[0].perms);
    ASSERT_EQ(0, strcmp(ZOO_READ_ACL_UNSAFE.data[0].id.id, acl.data[0].id.id));
    ASSERT_EQ(0, strcmp(ZOO_READ_ACL_UNSAFE.data[0].id.scheme, acl.data[0].id.scheme));

    INFOR_LOG("使用批量接口创建[%u]个节点,节点的名称为[node_*],数据为[data_*].", COUNT);
    MultiOps multi_ops = move(zk_manager.CreateMultiOps());
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string node_name = CppString::GetArgs("node_%u", i);
        string node_data = CppString::GetArgs("data_%u", i);
        multi_ops.AddCreateOp(node_name, node_data);
        multi_ops.AddSetOp(node_name, node_data, -1);
    }

    vector<zoo_op_result_t> results;
    ASSERT_EQ(ZOK, zk_manager.Multi(multi_ops, results));

    INFOR_LOG("获得所有子节点数据.");
    map<string, ValueStat> children_value;
    ASSERT_EQ(ZOK, zk_manager.GetChildrenValue("", children_value));
    ASSERT_EQ(COUNT * 2 - 1, children_value.size());

    for (auto it = children_value.begin(); it != children_value.end(); ++it)
    {
        if (it->first[0] == 'n')
        {
            // node_*
            uint32_t node_id = CppString::FromString<uint32_t>(it->first.substr(sizeof("node_") - 1));
            string node_data = CppString::GetArgs("data_%u", node_id);
            ASSERT_EQ(node_data, it->second.value);
            ASSERT_EQ(1, it->second.stat.version);
        }
        else
        {
            ASSERT_TRUE(it->first == it->second.value);
        }
    }
}

// ZookeeperManager异步API测试
TEST(ZooKeeper, DISABLED_ZkManagerAsyncTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    const uint32_t COUNT = 3;
    INFOR_LOG("在根节点下创建[%u]个子节点，名字和值为数字.", COUNT);
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string node_name = CppString::ToString(i);
        ASYNC_BEGIN;
        ASSERT_EQ(ZOK, zk_manager.ACreate(node_name, node_name,
                                          make_shared<StringCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const char *value)
        {
            static_cast<void>(zookeeper_manager);

            EXPECT_EQ(ZOK, rc);
            EXPECT_EQ(zk_manager.ChangeToAbsPath(node_name), value);

            NOTIFY_SYNC;
        })));
        WATI_SYNC;
    }

    string path_to_delete = CppString::ToString(COUNT - 2);
    INFOR_LOG("删除节点[%s].", path_to_delete.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.ADelete(path_to_delete, -1, make_shared<VoidCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("节点[%s]不存在了.", path_to_delete.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AExists(path_to_delete, make_shared<StatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZNONODE, rc);
        EXPECT_EQ(NULL, stat);

        NOTIFY_SYNC;
    })));
    sync_lock_u.lock();
    while (!done)
    {
        sync_cond.wait(sync_lock_u);
    }
    sync_lock_u.unlock();

    INFOR_LOG("获得根目录下所有节点，还有[%u]个.", COUNT - 1);
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGetChildren(TEST_ROOT_PATH, make_shared<StringsStatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const String_vector *children, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(static_cast<int32_t>(COUNT - 1), children->count);
        EXPECT_EQ(NULL, stat);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("再调用一次，不能有内存泄露.");
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGetChildren(TEST_ROOT_PATH, make_shared<StringsStatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const String_vector *children, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(static_cast<int32_t>(COUNT - 1), children->count);
        EXPECT_EQ(NULL, stat);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    string path_to_op = CppString::ToString(COUNT - 1);
    INFOR_LOG("节点[%s]存在.", path_to_op.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AExists(path_to_op, make_shared<StatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(0, stat->version);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("获得节点[%s]数据.", path_to_op.c_str());
    int last_version;
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGet(path_to_op, make_shared<DataCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const char *value, int value_len, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        string node_data(value, value_len);
        EXPECT_EQ(1, value_len);
        EXPECT_EQ(0, stat->version);
        last_version = stat->version;
        EXPECT_EQ(0, strcmp(path_to_op.c_str(), node_data.c_str()));

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    string new_data = "new_data";
    INFOR_LOG("设置节点[%s]数据为[%s].", path_to_op.c_str(), new_data.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.ASet(path_to_op, new_data.c_str(), last_version, make_shared<StatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(last_version + 1, stat->version);
        last_version = stat->version;

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("再次获得节点[%s]数据.", path_to_op.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGet(path_to_op, make_shared<DataCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const char *value, int value_len, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        string node_data(value, value_len);
        EXPECT_EQ(static_cast<int32_t>(new_data.size()), value_len);
        EXPECT_EQ(last_version, stat->version);
        EXPECT_EQ(0, strcmp(new_data.c_str(), node_data.c_str()));

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("设置回节点[%s]数据为[%s].", path_to_op.c_str(), path_to_op.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.ASet(path_to_op, path_to_op.c_str(), last_version, make_shared<StatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(last_version + 1, stat->version);
        last_version = stat->version;

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("获得节点[%s]ACL数据.", path_to_op.c_str());
    int last_aversion;
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGetAcl(path_to_op, make_shared<AclCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, ACL_vector *acl, Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(0, stat->aversion);
        last_aversion = stat->aversion;
        EXPECT_EQ(ZOO_OPEN_ACL_UNSAFE.count, acl->count);
        EXPECT_EQ(ZOO_OPEN_ACL_UNSAFE.data[0].perms, acl->data[0].perms);
        EXPECT_EQ(0, strcmp(ZOO_OPEN_ACL_UNSAFE.data[0].id.id, acl->data[0].id.id));
        EXPECT_EQ(0, strcmp(ZOO_OPEN_ACL_UNSAFE.data[0].id.scheme, acl->data[0].id.scheme));

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("设置节点[%s]ACL数据.", path_to_op.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.ASetAcl(path_to_op, last_aversion, &ZOO_READ_ACL_UNSAFE, make_shared<VoidCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("再次获得节点[%s]ACL数据.", path_to_op.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGetAcl(path_to_op, make_shared<AclCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, ACL_vector *acl, Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(1, stat->aversion);
        last_aversion = stat->aversion;
        EXPECT_EQ(ZOO_READ_ACL_UNSAFE.count, acl->count);
        EXPECT_EQ(ZOO_READ_ACL_UNSAFE.data[0].perms, acl->data[0].perms);
        EXPECT_EQ(0, strcmp(ZOO_READ_ACL_UNSAFE.data[0].id.id, acl->data[0].id.id));
        EXPECT_EQ(0, strcmp(ZOO_READ_ACL_UNSAFE.data[0].id.scheme, acl->data[0].id.scheme));

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("使用批量接口创建[%u]个节点,节点的名称为[node_*],数据为[data_*].", COUNT);
    shared_ptr<MultiOps> multi_ops = make_shared<MultiOps>(&zk_manager);
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string node_name = CppString::GetArgs("node_%u", i);
        string node_data = CppString::GetArgs("data_%u", i);
        multi_ops->AddCreateOp(node_name, node_data);
    }

    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AMulti(multi_ops, make_shared<MultiCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, std::shared_ptr<MultiOps> &multi_ops, std::shared_ptr<std::vector<zoo_op_result_t>> &multi_results)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(multi_ops->Size(), multi_results->size());

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("获得所有子节点数据.");
    map<string, ValueStat> children_value;
    ASSERT_EQ(ZOK, zk_manager.GetChildrenValue("", children_value));
    ASSERT_EQ(COUNT * 2 - 1, children_value.size());

    for (auto it = children_value.begin(); it != children_value.end(); ++it)
    {
        if (it->first[0] == 'n')
        {
            // node_*
            uint32_t node_id = CppString::FromString<uint32_t>(it->first.substr(sizeof("node_") - 1));
            string node_data = CppString::GetArgs("data_%u", node_id);
            ASSERT_EQ(node_data, it->second.value);
            ASSERT_EQ(0, it->second.stat.version);
        }
        else
        {
            ASSERT_TRUE(it->first == it->second.value);
        }
    }
}

// ZookeeperManager重连测试，包含同步注册Watcher和临时节点
TEST(ZooKeeper, DISABLED_ZkManagerReconnectTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    int last_global_watcher_type;
    int last_global_watcher_state;
    string last_global_watcher_path;

    ZookeeperManager zk_manager;
    zk_manager.Init(ZK_HOST, TEST_ROOT_PATH);

    INFOR_LOG("开始连接.");
    uint32_t expire_second = 5;
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        string type_str = GetEventTypeStr(type);

        if (path != NULL)
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p][%s].",
                      __FUNCTION__, type, type_str.c_str(), state, path, path);
        }
        else
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p].",
                      __FUNCTION__, type, type_str.c_str(), state, path);
        }

        last_global_watcher_type = type;
        last_global_watcher_state = state;
        if (path == NULL)
        {
            last_global_watcher_path.clear();
        }
        else
        {
            last_global_watcher_path = path;
        }

        NOTIFY_SYNC;

        return false;
    }), expire_second * 1000, 3000));
    WATI_SYNC;

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    const string GET_WATCHER_NODE = "get_watcher_node";

    INFOR_LOG("创建节点[%s].", GET_WATCHER_NODE.c_str());
    ASSERT_EQ(ZOK, zk_manager.Create(GET_WATCHER_NODE, ""));

    INFOR_LOG("在[%s]上使用Get注册全局Watcher.", GET_WATCHER_NODE.c_str());
    string node_data(MAX_DATA_LEN, '\0');
    int buflen = node_data.size() - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(GET_WATCHER_NODE, const_cast<char *>(node_data.data()), &buflen, NULL, 1));
    node_data[buflen] = '\0';
    ASSERT_EQ(0, strcmp(node_data.c_str(), ""));

    string new_data = "new_data";
    INFOR_LOG("修改[%s]数据为[%s]，触发全局Watcher.", GET_WATCHER_NODE.c_str(), new_data.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(GET_WATCHER_NODE, new_data, -1, NULL));
    WATI_SYNC;

    ASSERT_EQ(ZOO_CHANGED_EVENT, last_global_watcher_type);
    ASSERT_EQ(zk_manager.ChangeToAbsPath(GET_WATCHER_NODE), last_global_watcher_path);

    buflen = node_data.size() - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(GET_WATCHER_NODE, const_cast<char *>(node_data.data()), &buflen));
    node_data[buflen] = '\0';
    ASSERT_EQ(0, strcmp(node_data.c_str(), new_data.c_str()));

    INFOR_LOG("使用iptables断开连接.");
    DisableZkLink();
    for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
    {
        INFOR_LOG("等待%d秒.", remain_second);
        sleep(1);
    }

    INFOR_LOG("使用iptables重新连接.");
    ASYNC_BEGIN;
    EnableZkLink();
    WATI_SYNC;

    new_data = "new_data2";
    INFOR_LOG("修改[%s]数据为[%s]，触发全局Watcher.", GET_WATCHER_NODE.c_str(), new_data.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(GET_WATCHER_NODE, new_data, -1, NULL));
    WATI_SYNC;
}

// ZookeeperManager同步API带自定义Watcher测试
TEST(ZooKeeper, DISABLED_ZkManagerSyncCustomWatcherTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string WATCHER_PATH = TEST_ROOT_PATH + "/watcher_test";
    INFOR_LOG("Exist注册一个节点不存在的Watcher,路径[%s],在节点创建后和删除后会调用.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZNONODE, zk_manager.Exists(WATCHER_PATH, NULL,
                                        make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                        int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        INFOR_LOG("触发Watcher,type[%s].", GetEventTypeStr(type).c_str());

        EXPECT_EQ(ZOO_CONNECTED_STATE, state);
        EXPECT_EQ(0, strcmp(path, WATCHER_PATH.c_str()));
        if (type == ZOO_CREATED_EVENT)
        {
            // 继续Watcher
            NOTIFY_SYNC;
            return false;
        }
        else if (type == ZOO_DELETED_EVENT)
        {
            // 停止Watcher
            NOTIFY_SYNC;
            return true;
        }
        else
        {
            EXPECT_TRUE(false);
        }

        NOTIFY_SYNC;

        return false;
    })));

    INFOR_LOG("创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;

    INFOR_LOG("再次创建节点[%s],不触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));

    INFOR_LOG("Get注册Watcher测试,注册到[%s].", WATCHER_PATH.c_str());
    char buf[MAX_DATA_LEN];
    int buflen = MAX_DATA_LEN - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(WATCHER_PATH, buf, &buflen, NULL, make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                                                    int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        INFOR_LOG("触发Watcher,type[%s].", GetEventTypeStr(type).c_str());

        EXPECT_EQ(ZOO_CONNECTED_STATE, state);
        EXPECT_EQ(0, strcmp(path, WATCHER_PATH.c_str()));
        if (type == ZOO_CHANGED_EVENT)
        {
            // 继续Watcher
            NOTIFY_SYNC;
            return false;
        }
        else if (type == ZOO_DELETED_EVENT)
        {
            // 删除节点，停止Watcher
            NOTIFY_SYNC;
            return true;
        }
        else
        {
            EXPECT_TRUE(false);
        }

        NOTIFY_SYNC;

        return false;
    })));

    INFOR_LOG("修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "123", -1));
    WATI_SYNC;

    INFOR_LOG("再次修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "1234", -1));
    WATI_SYNC;

    INFOR_LOG("注册子节点事件[%s].", WATCHER_PATH.c_str());
    ScopedStringVector children;
    ASSERT_EQ(ZOK, zk_manager.GetChildren(WATCHER_PATH, children, make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                                                  int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        INFOR_LOG("触发Watcher,type[%s].", GetEventTypeStr(type).c_str());

        EXPECT_EQ(ZOO_CONNECTED_STATE, state);
        EXPECT_EQ(0, strcmp(path, WATCHER_PATH.c_str()));
        EXPECT_TRUE(type == ZOO_CHILD_EVENT || type == ZOO_DELETED_EVENT);

        NOTIFY_SYNC;
        return false;
    })));

    static const uint32_t COUNT = 10;
    INFOR_LOG("操作节点[%s]的子节点,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("创建节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Create(child_path, ""));
    }
    WATI_SYNC;

    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("删除节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Delete(child_path, -1));
    }
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;
}

// ZookeeperManager异步API带自定义Watcher测试
TEST(ZooKeeper, DISABLED_ZkManagerAsyncCustomWatcherTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string WATCHER_PATH = TEST_ROOT_PATH + "/watcher_test";
    INFOR_LOG("Exist注册一个节点不存在的Watcher,路径[%s],在节点创建后和删除后会调用.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AExists(WATCHER_PATH,
                                      make_shared<StatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);
        static_cast<void>(stat);

        EXPECT_EQ(ZNONODE, rc);

        NOTIFY_SYNC;
    }),
                                      make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        INFOR_LOG("触发Watcher,type[%s].", GetEventTypeStr(type).c_str());

        EXPECT_EQ(ZOO_CONNECTED_STATE, state);
        EXPECT_EQ(0, strcmp(path, WATCHER_PATH.c_str()));
        if (type == ZOO_CREATED_EVENT)
        {
            // 继续Watcher
            NOTIFY_SYNC;
            return false;
        }
        else if (type == ZOO_DELETED_EVENT)
        {
            // 停止Watcher
            NOTIFY_SYNC;
            return true;
        }
        else
        {
            EXPECT_TRUE(false);
        }

        NOTIFY_SYNC;

        return false;
    })));
    WATI_SYNC;

    INFOR_LOG("创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;

    INFOR_LOG("再次创建节点[%s],不触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));

    INFOR_LOG("Get注册Watcher测试,注册到[%s].", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGet(WATCHER_PATH, make_shared<DataCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const char *value, int value_len, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        string node_data(value, value_len);
        EXPECT_EQ(0, value_len);
        EXPECT_EQ(0, stat->version);
        EXPECT_EQ(0, strcmp("", node_data.c_str()));

        NOTIFY_SYNC;
    }), make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                        int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        INFOR_LOG("触发Watcher,type[%s].", GetEventTypeStr(type).c_str());

        EXPECT_EQ(ZOO_CONNECTED_STATE, state);
        EXPECT_EQ(0, strcmp(path, WATCHER_PATH.c_str()));
        if (type == ZOO_CHANGED_EVENT)
        {
            // 继续Watcher
            NOTIFY_SYNC;
            return false;
        }
        else if (type == ZOO_DELETED_EVENT)
        {
            // 停止Watcher
            NOTIFY_SYNC;
            return true;
        }
        else
        {
            EXPECT_TRUE(false);
        }

        NOTIFY_SYNC;

        return false;
    })));
    WATI_SYNC;

    INFOR_LOG("修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "123", -1));
    WATI_SYNC;

    INFOR_LOG("再次修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "1234", -1));
    WATI_SYNC;

    INFOR_LOG("注册子节点事件[%s].", WATCHER_PATH.c_str());
    ScopedStringVector children;
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGetChildren(WATCHER_PATH, make_shared<StringsStatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const String_vector *children, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);
        static_cast<void>(stat);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(0, children->count);

        NOTIFY_SYNC;
    }), make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                        int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        INFOR_LOG("触发Watcher,type[%s].", GetEventTypeStr(type).c_str());

        EXPECT_EQ(ZOO_CONNECTED_STATE, state);
        EXPECT_EQ(0, strcmp(path, WATCHER_PATH.c_str()));
        EXPECT_TRUE(type == ZOO_CHILD_EVENT || type == ZOO_DELETED_EVENT);

        NOTIFY_SYNC;
        return false;
    })));
    WATI_SYNC;

    static const uint32_t COUNT = 10;
    INFOR_LOG("操作节点[%s]的子节点,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("创建节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Create(child_path, ""));
    }
    WATI_SYNC;

    INFOR_LOG("获得节点[%s]的子节点.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AGetChildren(WATCHER_PATH, make_shared<StringsStatCompletionFunType>([&](ZookeeperManager &zookeeper_manager, int rc, const String_vector *children, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(static_cast<int32_t>(COUNT), children->count);
        EXPECT_EQ(static_cast<int32_t>(COUNT), stat->numChildren);
        EXPECT_EQ(2, stat->version);

        NOTIFY_SYNC;
    }), make_shared<WatcherFunType>(), true));
    WATI_SYNC;

    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("删除节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Delete(child_path, -1));
    }
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发2次Watcher，均为DELETE事件.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;
}

// ZookeeperManager 异常测试
TEST(ZooKeeper, DISABLED_ZkManagerExceptionTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    // 连接超时
    DisableZkLink();
    ZookeeperManager zk_manager;
    ASSERT_EQ(ZBADARGUMENTS, zk_manager.Init(ZK_HOST, ""));
    ASSERT_EQ(ZOK, zk_manager.Init(ZK_HOST, TEST_ROOT_PATH));

    uint32_t expire_second = 5;
    INFOR_LOG("%u秒连接超时.", expire_second);
    ASSERT_EQ(ZOPERATIONTIMEOUT, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                                    int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        static_cast<void>(type);
        static_cast<void>(state);
        static_cast<void>(path);

        NOTIFY_SYNC;

        return false;
    }), expire_second * 1000, 3000));
    ASYNC_BEGIN;
    EnableZkLink();
    WATI_SYNC;

    INFOR_LOG("重连成功.");

    shared_ptr<MultiOps> multi_ops = make_shared<MultiOps>(&zk_manager);
    ASSERT_EQ(ZBADARGUMENTS, zk_manager.AMulti(multi_ops, make_shared<MultiCompletionFunType>()));

    vector<zoo_op_result_t> results;
    ASSERT_EQ(ZBADARGUMENTS, zk_manager.Multi(*multi_ops, results));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string EXCEPTION_PATH = TEST_ROOT_PATH + "/exception_test";

    INFOR_LOG("无效的路径.");
    ASSERT_EQ(ZBADARGUMENTS, zk_manager.Create("a/", ""));

    INFOR_LOG("空节点，为根节点，已经存在.");
    ASSERT_EQ(ZNODEEXISTS, zk_manager.Create("", ""));

    INFOR_LOG("Get的时候缓冲区为NULL.");
    ASSERT_EQ(ZOK, zk_manager.Create(EXCEPTION_PATH, ""));
    ASSERT_EQ(ZBADARGUMENTS, zk_manager.Get(EXCEPTION_PATH, NULL, 0));

    INFOR_LOG("删除节点.");
    ASSERT_EQ(ZOK, zk_manager.Delete(EXCEPTION_PATH, -1));

}

// ZookeeperManager 同步全局Watcher测试
TEST(ZooKeeper, DISABLED_ZkManagerSyncGlobalWatcherTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        string type_str = GetEventTypeStr(type);

        if (path != NULL)
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p][%s].",
                      __FUNCTION__, type, type_str.c_str(), state, path, path);
        }
        else
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p].",
                      __FUNCTION__, type, type_str.c_str(), state, path);
        }

        if (type == ZOO_DELETED_EVENT)
        {
            // 停止Watcher
            NOTIFY_SYNC;
            return true;
        }

        NOTIFY_SYNC;
        return false;
    }), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string WATCHER_PATH = TEST_ROOT_PATH + "/watcher_test";
    INFOR_LOG("Exist注册一个节点不存在的Watcher,路径[%s],在节点创建后和删除后会调用.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZNONODE, zk_manager.Exists(WATCHER_PATH, NULL, 1));

    INFOR_LOG("创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;

    INFOR_LOG("再次创建节点[%s],不触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));

    INFOR_LOG("Get注册Watcher测试,注册到[%s].", WATCHER_PATH.c_str());
    char buf[MAX_DATA_LEN];
    int buflen = MAX_DATA_LEN - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(WATCHER_PATH, buf, &buflen, NULL, 1));

    INFOR_LOG("修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "123", -1));
    WATI_SYNC;

    INFOR_LOG("再次修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "1234", -1));
    WATI_SYNC;

    INFOR_LOG("注册子节点事件[%s].", WATCHER_PATH.c_str());
    ScopedStringVector children;
    ASSERT_EQ(ZOK, zk_manager.GetChildren(WATCHER_PATH, children, 1));

    static const uint32_t COUNT = 10;
    INFOR_LOG("操作节点[%s]的子节点,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("创建节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Create(child_path, ""));
    }
    WATI_SYNC;

    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("删除节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Delete(child_path, -1));
    }
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;
}

// ZookeeperManager 同步全局Watcher测试：Exists在删除节点后还会执行重注册
TEST(ZooKeeper, DISABLED_ZkManagerSyncGlobalWatcherExistsTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        string type_str = GetEventTypeStr(type);

        if (path != NULL)
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p][%s].",
                      __FUNCTION__, type, type_str.c_str(), state, path, path);
        }
        else
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p].",
                      __FUNCTION__, type, type_str.c_str(), state, path);
        }

        NOTIFY_SYNC;
        return false;
    }), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string WATCHER_PATH = TEST_ROOT_PATH + "/watcher_test";
    INFOR_LOG("Exist注册一个节点不存在的Watcher,路径[%s],在节点创建后和删除后会调用.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZNONODE, zk_manager.Exists(WATCHER_PATH, NULL, 1));

    INFOR_LOG("创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;

    INFOR_LOG("再次创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;
}

// 这次使用GetChildren接口进行注册，在节点删除后，不会重新注册.
TEST(ZooKeeper, ZkManagerSyncGlobalWatcherGetChildrenTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);
    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        string type_str = GetEventTypeStr(type);

        if (path != NULL)
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p][%s].",
                      __FUNCTION__, type, type_str.c_str(), state, path, path);
        }
        else
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p].",
                      __FUNCTION__, type, type_str.c_str(), state, path);
        }

        NOTIFY_SYNC;
        return false;
    }), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string WATCHER_PATH = TEST_ROOT_PATH + "/watcher_test";
    INFOR_LOG("创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));

    INFOR_LOG("关注子节点[%s].", WATCHER_PATH.c_str());
    ScopedStringVector children;
    ASSERT_EQ(ZOK, zk_manager.GetChildren(WATCHER_PATH, children, 1));

    string child_path = WATCHER_PATH + "/children";
    INFOR_LOG("创建子节点[%s],触发Watcher.", child_path.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Create(child_path, ""));
    WATI_SYNC;

    INFOR_LOG("删除子节点[%s],触发Watcher.", child_path.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(child_path, -1));
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;

    INFOR_LOG("创建节点[%s],不触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.Create(WATCHER_PATH, ""));

    INFOR_LOG("删除节点[%s],不触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
}

// ZookeeperManager 异步全局Watcher测试
TEST(ZooKeeper, DISABLED_ZkManagerAsyncGlobalWatcherTest)
{
    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        string type_str = GetEventTypeStr(type);

        if (path != NULL)
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p][%s].",
                      __FUNCTION__, type, type_str.c_str(), state, path, path);
        }
        else
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p].",
                      __FUNCTION__, type, type_str.c_str(), state, path);
        }

        if (type == ZOO_DELETED_EVENT)
        {
            // 停止Watcher
            NOTIFY_SYNC;
            return true;
        }

        NOTIFY_SYNC;
        return false;
    }), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string WATCHER_PATH = TEST_ROOT_PATH + "/watcher_test";
    INFOR_LOG("Exist注册一个节点不存在的Watcher,路径[%s],在节点创建后和删除后会调用.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.AExists(WATCHER_PATH, make_shared<StatCompletionFunType>(
        [&](ZookeeperManager &zookeeper_manager, int rc, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);
        static_cast<void>(stat);

        EXPECT_EQ(ZNONODE, rc);

        NOTIFY_SYNC;
    }), 1));
    WATI_SYNC;

    INFOR_LOG("创建节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.ACreate(WATCHER_PATH, WATCHER_PATH, make_shared<StringCompletionFunType>(
        [&](ZookeeperManager &zookeeper_manager, int rc, const char *value)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(zk_manager.ChangeToAbsPath(WATCHER_PATH), value);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.ADelete(WATCHER_PATH, -1, make_shared<VoidCompletionFunType>(
        [&](ZookeeperManager &zookeeper_manager, int rc)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;
    WATI_SYNC;

    INFOR_LOG("再次创建节点[%s],不触发Watcher.", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.ACreate(WATCHER_PATH, WATCHER_PATH, make_shared<StringCompletionFunType>(
        [&](ZookeeperManager &zookeeper_manager, int rc, const char *value)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(zk_manager.ChangeToAbsPath(WATCHER_PATH), value);

        NOTIFY_SYNC;
    })));
    WATI_SYNC;

    INFOR_LOG("Get注册Watcher测试,注册到[%s].", WATCHER_PATH.c_str());
    int last_version;
    ASSERT_EQ(ZOK, zk_manager.AGet(WATCHER_PATH, make_shared<DataCompletionFunType>(
        [&](ZookeeperManager &zookeeper_manager, int rc, const char *value, int value_len, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        string node_data(value, value_len);
        EXPECT_EQ(static_cast<int32_t>(WATCHER_PATH.size()), value_len);
        EXPECT_EQ(0, stat->version);
        last_version = stat->version;
        EXPECT_EQ(0, strcmp(WATCHER_PATH.c_str(), node_data.c_str()));

        NOTIFY_SYNC;
    }), 1));
    WATI_SYNC;

    INFOR_LOG("修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "123", -1));
    WATI_SYNC;

    INFOR_LOG("再次修改节点[%s]数据,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Set(WATCHER_PATH, "1234", -1));
    WATI_SYNC;

    INFOR_LOG("注册子节点事件[%s].", WATCHER_PATH.c_str());
    ASSERT_EQ(ZOK, zk_manager.AGetChildren(WATCHER_PATH, make_shared<StringsStatCompletionFunType>(
        [&](ZookeeperManager &zookeeper_manager, int rc, const String_vector *children, const Stat *stat)
    {
        static_cast<void>(zookeeper_manager);

        EXPECT_EQ(ZOK, rc);
        EXPECT_EQ(0, children->count);
        EXPECT_EQ(NULL, stat);

        NOTIFY_SYNC;
    }), 1));
    WATI_SYNC;

    static const uint32_t COUNT = 10;
    INFOR_LOG("操作节点[%s]的子节点,触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("创建节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Create(child_path, ""));
    }
    WATI_SYNC;

    ASYNC_BEGIN;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string child_path = WATCHER_PATH + "/" + CppString::ToString(i);
        INFOR_LOG("删除节点[%s],触发Watcher.", child_path.c_str());
        ASSERT_EQ(ZOK, zk_manager.Delete(child_path, -1));
    }
    WATI_SYNC;

    INFOR_LOG("删除节点[%s],触发Watcher.", WATCHER_PATH.c_str());
    ASYNC_BEGIN;
    ASSERT_EQ(ZOK, zk_manager.Delete(WATCHER_PATH, -1));
    WATI_SYNC;
}

// ZookeeperManager 额外接口测试
TEST(ZooKeeper, DISABLED_ZkManagerExtraInterfaceTest)
{
    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string DATA_NODE_PATH = "data_node";
    static const string DATA_NODE_DATA = "data";
    string real_path(MAX_PATH_LEN, '\0');

    ASSERT_EQ(ZOK, zk_manager.Create(DATA_NODE_PATH, DATA_NODE_DATA, &real_path));
    ASSERT_EQ(0, strcmp(zk_manager.ChangeToAbsPath(DATA_NODE_PATH).c_str(), real_path.c_str()));

    string get_data(MAX_DATA_LEN, '\0');
    Stat stat;
    ASSERT_EQ(ZOK, zk_manager.GetCString(DATA_NODE_PATH, get_data, &stat, 0));
    ASSERT_EQ(0, strcmp(get_data.c_str(), DATA_NODE_DATA.c_str()));
    ASSERT_EQ(0, stat.version);

    ASSERT_EQ(ZOK, zk_manager.GetCString(DATA_NODE_PATH, get_data, &stat, make_shared<WatcherFunType>()));
    ASSERT_EQ(0, strcmp(get_data.c_str(), DATA_NODE_DATA.c_str()));
    ASSERT_EQ(0, stat.version);
}

// ZookeeperManager 临时节点测试
TEST(ZooKeeper, DISABLED_ZkManagerEphemeralNodeTest)
{
    EnableZkLink();

    // 跟锁相关的变量
    bool done = false;
    mutex sync_lock;
    condition_variable sync_cond;
    unique_lock<mutex> sync_lock_u(sync_lock);
    sync_lock_u.unlock();

    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

    uint32_t expire_second = 5;
    INFOR_LOG("开始连接,超时时间%d秒.", expire_second);
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>([&](ZookeeperManager &zookeeper_manager,
                                                                      int type, int state, const char *path) -> bool
    {
        static_cast<void>(zookeeper_manager);
        string type_str = GetEventTypeStr(type);

        if (path != NULL)
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p][%s].",
                      __FUNCTION__, type, type_str.c_str(), state, path, path);
        }
        else
        {
            DEBUG_LOG("触发%s,type[%d],type_str[%s],state[%d],path[%p].",
                      __FUNCTION__, type, type_str.c_str(), state, path);
        }

        NOTIFY_SYNC;
        return false;
    }), expire_second * 1000, 3000));
    WATI_SYNC;

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string EPHEMERAL_PATH = TEST_ROOT_PATH + "/ephemeral_test";
    INFOR_LOG("创建临时节点.");
    ASSERT_EQ(ZOK, zk_manager.Create(EPHEMERAL_PATH, "", NULL, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL));

    INFOR_LOG("批量创建临时节点.");
    static const uint32_t COUNT = 10;
    MultiOps multi_ops = zk_manager.CreateMultiOps();
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string path = EPHEMERAL_PATH + CppString::ToString(i);
        multi_ops.AddCreateOp(path, path, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL);
    }
    vector<zoo_op_result_t> results;
    ASSERT_EQ(ZOK, zk_manager.Multi(multi_ops, results));

    INFOR_LOG("使用iptables断开连接.");
    DisableZkLink();
    for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
    {
        INFOR_LOG("等待%d秒.", remain_second);
        sleep(1);
    }

    INFOR_LOG("使用iptables重新连接.");
    ASYNC_BEGIN;
    EnableZkLink();
    WATI_SYNC;

    INFOR_LOG("获得临时节点,节点还存在.");
    string node_data(MAX_DATA_LEN, '\0');
    int data_len = node_data.size() - 1;
    ASSERT_EQ(ZOK, zk_manager.Get(EPHEMERAL_PATH, const_cast<char *>(node_data.c_str()), &data_len));
    node_data[data_len] = '\0';
    ASSERT_EQ(0, data_len);

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string path = EPHEMERAL_PATH + CppString::ToString(i);
        ASSERT_EQ(ZOK, zk_manager.Get(path, const_cast<char *>(node_data.c_str()), &data_len));
    }

    INFOR_LOG("删除临时节点.");
    ASSERT_EQ(ZOK, zk_manager.Delete(EPHEMERAL_PATH, -1));

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string path = EPHEMERAL_PATH + CppString::ToString(i);
        ASSERT_EQ(ZOK, zk_manager.Delete(path, -1));
    }

    // 下一个测试
    INFOR_LOG("超时后，临时节点的父节点也被删除，需要递归创建父节点，然后创建临时节点.");
    INFOR_LOG("创建临时节点.");
    ASSERT_EQ(ZOK, zk_manager.Create(EPHEMERAL_PATH, "", NULL, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL));

    INFOR_LOG("使用iptables断开连接.");
    DisableZkLink();
    for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
    {
        INFOR_LOG("等待%d秒.", remain_second);
        sleep(1);
    }

    INFOR_LOG("使用iptables重新连接.");
    ASYNC_BEGIN;
    EnableZkLink();

    INFOR_LOG("使用另一个连接删除临时节点的父节点,等待第一个连接恢复.");
    string data;
    Stat stat;
    {
        ZookeeperManager zk_manager2;
        zk_manager2.InitFromFile(ZK_CONFIG_FILE_PATH);
        zk_manager2.Connect(shared_ptr<WatcherFunType>(), expire_second * 1000, 3000);
        ASSERT_EQ(ZOK, zk_manager2.DeletePathRecursion(TEST_ROOT_PATH));
        WATI_SYNC;

        INFOR_LOG("原始连接恢复，临时节点已经重新创建了,版本号为1.");
        ASSERT_EQ(ZOK, zk_manager.GetCString(EPHEMERAL_PATH, data, &stat));
        ASSERT_EQ(0, stat.version);

        /* 下一个测试 */
        INFOR_LOG("注册临时节点后，别的连接删除了临时节点，原始连接将父目录递归删除，重连之后，不能再注册临时节点.");
        INFOR_LOG("另一个连接删除节点.");
        ASSERT_EQ(ZOK, zk_manager2.DeletePathRecursion(EPHEMERAL_PATH));
    }

    INFOR_LOG("原始连接将父目录递归删除.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("使用iptables断开连接.");
    DisableZkLink();
    for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
    {
        INFOR_LOG("等待%d秒.", remain_second);
        sleep(1);
    }

    INFOR_LOG("使用iptables重新连接.");
    ASYNC_BEGIN;
    EnableZkLink();
    WATI_SYNC;

    INFOR_LOG("原始连接恢复，临时节点不存在.");
    ASSERT_EQ(ZNONODE, zk_manager.GetCString(EPHEMERAL_PATH, data, &stat));

    /* 下一个测试 */
    INFOR_LOG("超时后，临时节点重注册失败的情况，反复尝试，直到成功.");
    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建临时节点.");
    ASSERT_EQ(ZOK, zk_manager.Create(EPHEMERAL_PATH, "", NULL, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL));

    INFOR_LOG("使用iptables断开连接.");
    DisableZkLink();
    for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
    {
        INFOR_LOG("等待%d秒.", remain_second);
        sleep(1);
    }

    INFOR_LOG("使用iptables重新连接.");
    ASYNC_BEGIN;
    EnableZkLink();

    INFOR_LOG("使用另一个连接删除临时节点的父节点,并且将他的父节点改成临时节点,于是第一个节点的临时节点就无法创建,等待第一个连接恢复.");
    {
        ZookeeperManager zk_manager2;
        zk_manager2.InitFromFile(ZK_CONFIG_FILE_PATH);
        zk_manager2.Connect(shared_ptr<WatcherFunType>(), expire_second * 1000, 3000);
        ASSERT_EQ(ZOK, zk_manager2.DeletePathRecursion(TEST_ROOT_PATH));
        ASSERT_EQ(ZOK, zk_manager2.Create(TEST_ROOT_PATH, "", NULL, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL));
        WATI_SYNC;

        for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
        {
            INFOR_LOG("等待%d秒.", remain_second);
            sleep(1);
        }
    }

    INFOR_LOG("第二个连接释放，删除临时根节点，第一个连接恢复临时节点的创建.");
    for (int32_t remain_second = 15; remain_second > 0; --remain_second)
    {
        INFOR_LOG("等待%d秒.", remain_second);
        sleep(1);
    }

    // TODO，这个目前不知道如何处理
//     INFOR_LOG("原始连接恢复，临时节点已经重新创建了,版本号为1.");
//     ASSERT_EQ(ZOK, zk_manager.GetCString(EPHEMERAL_PATH, data, &stat));
//     ASSERT_EQ(0, stat.version);
}

// ZookeeperManager 序列节点测试
TEST(ZooKeeper, DISABLED_ZkManagerSequenceNodeTest)
{
    ZookeeperManager zk_manager;
    zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);


    INFOR_LOG("开始连接.");
    ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), 30000, 3000));

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

    static const string SEQUENCE_PATH = TEST_ROOT_PATH + "/sequence_test";
    INFOR_LOG("创建序列节点.");
    string real_path(128, '\0');
    ASSERT_EQ(ZOK, zk_manager.Create(SEQUENCE_PATH, "", &real_path, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE));
    INFOR_LOG("成功创建序列节点[%s],size[%lu].", real_path.c_str(), real_path.size());
    ASSERT_NE(real_path, SEQUENCE_PATH);
    ASSERT_LT(SEQUENCE_PATH.size(), strlen(real_path.c_str()));
}

extern "C"
{
    void free_completions(zhandle_t *zh, int callCompletion, int reason);
}

// ZookeeperManager ClientId测试
TEST(ZooKeeper, DISABLED_ZkManagerClientIdTest)
{
    clientid_t client_id;
    uint32_t expire_second = 1;        // 设置Session超时时间
    static const string SEQUENCE_PATH = TEST_ROOT_PATH + "/sequence_ephemeral_test";
    ZookeeperManager zk_manager_global;
    zk_manager_global.InitFromFile(ZK_CONFIG_FILE_PATH);

    INFOR_LOG("全局客户端开始连接.");
    ASSERT_EQ(ZOK, zk_manager_global.Connect(make_shared<WatcherFunType>(), expire_second * 1000, 3000));
    string old_real_path(128, '\0');

    INFOR_LOG("清除数据，删除根节点.");
    ASSERT_EQ(ZOK, zk_manager_global.DeletePathRecursion(TEST_ROOT_PATH));

    INFOR_LOG("创建根节点.");
    ASSERT_EQ(ZOK, zk_manager_global.CreatePathRecursion(TEST_ROOT_PATH));

    {
        ZookeeperManager zk_manager;
        zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH);

        INFOR_LOG("临时客户端开始连接.");
        ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), expire_second * 1000, 3000));

        INFOR_LOG("创建根节点.");
        ASSERT_EQ(ZOK, zk_manager.CreatePathRecursion(TEST_ROOT_PATH));

        INFOR_LOG("创建临时序列节点.");
        ASSERT_EQ(ZOK, zk_manager.Create(SEQUENCE_PATH, "", &old_real_path, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, true));
        INFOR_LOG("成功创建临时序列节点[%s],size[%lu].", old_real_path.c_str(), old_real_path.size());

        string real_path(128, '\0');
        INFOR_LOG("另一个客户端也创建临时序列节点.");
        ASSERT_EQ(ZOK, zk_manager_global.Create(SEQUENCE_PATH, "", &real_path, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, true));
        INFOR_LOG("客户端2成功创建临时序列节点[%s],size[%lu].", real_path.c_str(), real_path.size());

        client_id = *zk_manager.GetClientID();
        INFOR_LOG("获得ClinetID[%ld].", client_id.client_id);

        INFOR_LOG("设置退出不告诉Server,只有在状态为ZOO_CONNECTED_STATE的时候才会向Server发送请求.");
        zk_manager.GetHandler()->state = ZOO_AUTH_FAILED_STATE;
        close(zk_manager.GetHandler()->fd);
    }

    {
        ZookeeperManager zk_manager;
        zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH, &client_id);

        INFOR_LOG("使用ClinetID重新连接.");
        ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), expire_second * 1000, 3000));

        INFOR_LOG("ClinetID必须和之前是一样的.");
        const clientid_t *p_curr_client_id = zk_manager.GetClientID();
        ASSERT_EQ(client_id.client_id, p_curr_client_id->client_id);
        ASSERT_EQ(0, memcmp(client_id.passwd, p_curr_client_id->passwd, sizeof(client_id.passwd)));

        INFOR_LOG("恢复临时序列节点.");
        string real_path(128, '\0');
        ASSERT_EQ(ZOK, zk_manager.Create(SEQUENCE_PATH, "1", &real_path, &ZOO_OPEN_ACL_UNSAFE, ZOO_SEQUENCE | ZOO_EPHEMERAL, true));
        INFOR_LOG("成功恢复临时序列节点[%s],size[%lu].", real_path.c_str(), real_path.size());
        ASSERT_EQ(0, strcmp(old_real_path.c_str(), real_path.c_str()));

        INFOR_LOG("检查临时节点数量，应该只有2个.");
        ScopedStringVector children;
        ASSERT_EQ(ZOK, zk_manager.GetChildren(TEST_ROOT_PATH, children));
        ASSERT_EQ(2, children.count);

        INFOR_LOG("检查节点内容，应该已经被改成1了，并且节点的归属还是当前的client.");
        string node_data(MAX_DATA_LEN, '\0');
        int data_len = node_data.size() - 1;
        Stat node_stat;
        ASSERT_EQ(ZOK, zk_manager.Get(real_path, const_cast<char *>(node_data.c_str()), &data_len, &node_stat));
        ASSERT_EQ(0, strcmp(node_data.c_str(), "1"));
        ASSERT_EQ(client_id.client_id, node_stat.ephemeralOwner);

        INFOR_LOG("设置退出不告诉Server.");
        zk_manager.GetHandler()->state = ZOO_AUTH_FAILED_STATE;
        close(zk_manager.GetHandler()->fd);
    }

    {
        INFOR_LOG("等待Session超时.");
        for (int32_t remain_second = expire_second + 5; remain_second > 0; --remain_second)
        {
            INFOR_LOG("等待%d秒.", remain_second);
            sleep(1);
        }

        ZookeeperManager zk_manager;
        zk_manager.InitFromFile(ZK_CONFIG_FILE_PATH, &client_id);

        INFOR_LOG("使用ClinetID重新连接,会提示超时.");
        ASSERT_EQ(ZOK, zk_manager.Connect(make_shared<WatcherFunType>(), expire_second * 1000, 3000));

        INFOR_LOG("获得子节点,应该不存在了.");
        string node_data(MAX_DATA_LEN, '\0');
        int data_len = node_data.size() - 1;
        Stat node_stat;
        ASSERT_EQ(ZNONODE, zk_manager.Get(SEQUENCE_PATH, const_cast<char *>(node_data.c_str()), &data_len, &node_stat));
    }
}

#endif
