#ifndef __CYGWIN__
#include <iostream>
#include <string>
#include <memory>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/comparator.h>
#include "leveldb/cache.h"

#include <CppString.h>
#include <CppTime.h>

#include "gtest/gtest.h"

using namespace std;

// 跟着doc中的指引一步一步来学习LevelDB
TEST(LevelDbTest, IntroCreateDB)
{
    {
        /* 创建一个DB实例 */
        leveldb::DB *db;
        leveldb::Options options;
        options.create_if_missing = true;

        // 如果目录无权限打开，则提示错误
        leveldb::Status status = leveldb::DB::Open(options, "/DirNotExists", &db);

        // 错误可以使用status.ok()来判断，为真表示成功，为假表示失败
        EXPECT_FALSE(status.ok());
        EXPECT_EQ(NULL, db);

        // 打开一个目录，这个目录里存放LevelDB相关的数据文件
        status = leveldb::DB::Open(options, "data/levelDbTest", &db);
        EXPECT_TRUE(status.ok());
        EXPECT_TRUE(db != NULL);

        /* 首次创建，目录里有如下文件
        godmoon@devubuntu:~/study/project/moon/MoonLib/gtest/data/levelDbTest$ ll
        总用量 40
        -rw-rw-r-- 1 godmoon godmoon  170  1月 22 15:08 119087.ldb
        -rw-rw-r-- 1 godmoon godmoon  232  1月 22 15:09 119093.ldb
        -rw-rw-r-- 1 godmoon godmoon  376  1月 22 15:09 119118.ldb
        -rw-rw-r-- 1 godmoon godmoon  170  1月 22 15:09 119121.ldb
        -rw-rw-r-- 1 godmoon godmoon 1665  1月 22 15:09 119124.ldb
        -rw-rw-r-- 1 godmoon godmoon  240  1月 22 15:09 119125.log
        -rw-rw-r-- 1 godmoon godmoon   16  1月 22 15:09 CURRENT
        -rw-r--r-- 1 godmoon godmoon    0  1月 22 11:06 LOCK
        -rw-rw-r-- 1 godmoon godmoon  556  1月 22 15:09 LOG
        -rw-rw-r-- 1 godmoon godmoon  741  1月 22 15:09 LOG.old
        -rw-rw-r-- 1 godmoon godmoon  273  1月 22 15:09 MANIFEST-119123
        **/

        // 关闭DB
        delete db;
    }

    {
        // 如果是要创建一个DB，在目录中DB如果已经存在则报错，那么置error_if_exists为true
        leveldb::DB *db;
        leveldb::Options options;
        options.error_if_exists = true;

        // 因为DB已经存在，所以报错了
        leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
        EXPECT_FALSE(status.ok());

        delete db;
    }

    // 如果不关闭DB会怎么样？
    // 结果：第二次打开的时候会失败，一个DB只能由一个操作实例打开，LevelDB通过系统锁避免被多次打开。
    // LevelDB实例是线程安全的，内部有多线程同步处理，它可以被多个线程共享，多个线程的操作不需要额外的多线程同步过程。
    //     while (true)
    //     {
    //         // 如果是要创建一个DB，在目录中DB如果已经存在则报错，那么置error_if_exists为true
    //         leveldb::DB *db;
    //         leveldb::Options options;
    //         options.create_if_missing = true;
    // 
    //         // 因为DB已经存在，所以报错了
    //         // 详细错误文本信息可以通过status.ToString()获得
    //         leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    //         cerr << status.ToString();   // IO error: lock data/levelDbTest/LOCK: already held by processsrc/LevelDbTest.cpp:71: Failure
    //         EXPECT_TRUE(status.ok());
    // 
    //         // delete db;
    //     }

    // 使用unique_ptr让DB自动释放
    static uint32_t COUNT = 10;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 如果是要创建一个DB，在目录中DB如果已经存在则报错，那么置error_if_exists为true
        leveldb::DB *db;
        leveldb::Options options;
        options.create_if_missing = true;

        // 因为DB已经存在，所以报错了
        // 详细错误文本信息可以通过status.ToString()获得
        leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
        EXPECT_TRUE(status.ok());
        unique_ptr<leveldb::DB> uniqueDb(db);
    }
}

TEST(LevelDbTest, WriteAndRead)
{
    // 普通读写操作
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    // levelDB的Key和Value的格式均为leveldb::Slice，它的构造方式和std::string很类似，可通过字符串或者string进行构造
    // 它比std::string要轻量，操作起来效率更高，因为内部存储的只是外部的一个字符串指针，复制的时候不需要大数据拷贝
    // 但是同时要注意因为保存的是外部字符串的指针，在使用Slice的时候要确保在外部的指针的生命周期内
    string str("123");
    leveldb::Slice slice(str);
    EXPECT_EQ(str.data(), slice.data());

    // 写COUNT个数据，再把他们读出来，最后删掉
    static uint32_t COUNT = 10;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 使用默认的写选项即可
        status = db->Put(leveldb::WriteOptions(),
                         leveldb::Slice(reinterpret_cast<const char *>(&i), sizeof(i)),
                         CppString::ToString(i));
        EXPECT_TRUE(status.ok());
    }

    // 遍历所有的Key
    // 将数据读取出来
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        string value;
        status = db->Get(leveldb::ReadOptions(),
                         leveldb::Slice(reinterpret_cast<const char *>(&i), sizeof(i)),
                         &value);
        EXPECT_TRUE(status.ok());
        EXPECT_EQ(value, CppString::ToString(i));
    }

    // 删除所有数据
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 删除数据也使用默认的写选项
        status = db->Delete(leveldb::WriteOptions(),
                            leveldb::Slice(reinterpret_cast<const char *>(&i), sizeof(i)));
        EXPECT_TRUE(status.ok());
    }
}

TEST(LevelDbTest, WriteBatch)
{
    // 原子操作、批量操作：WriteBatch
    // 需要包含头文件：#include "leveldb/write_batch.h"
    // WriteBatch只支持写和删操作，即只支持写入操作，不能在里面Get
    // WriteBatch的好处：
    //      支持原子操作，如果失败，整个事务不会执行
    //      支持批量操作，节约写入时间
    // WriteBatch的使用风险：
    //      非线程安全，多线程使用时需要自己做线程的数据保护
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    string key1 = "key1";
    string key2 = "key2";

    // 构造key1
    status = db->Put(leveldb::WriteOptions(), key1, "value");
    EXPECT_TRUE(status.ok());

    // 删除一个Key，将它的值动到另一个Key
    string value;
    status = db->Get(leveldb::ReadOptions(), key1, &value);
    EXPECT_TRUE(status.ok());

    // 将操作都放到WriteBatch对象中
    leveldb::WriteBatch batch;
    batch.Delete(key1);
    batch.Delete("NotFount");       // 可以删除不存在的Key
    batch.Put(key2, value);

    // 可以写完马上删
    batch.Put(key1, value);
    batch.Delete(key1);

    // 最后进行DB操作，执行的顺序与放入batch中的操作顺序相同
    status = db->Write(leveldb::WriteOptions(), &batch);
    EXPECT_TRUE(status.ok());

    status = db->Get(leveldb::ReadOptions(), key1, &value);
    EXPECT_FALSE(status.ok());
    EXPECT_TRUE(status.IsNotFound());
    // NotFound: 
    // cout << status.ToString() << endl;

    status = db->Delete(leveldb::WriteOptions(), key1);
    EXPECT_TRUE(status.ok());

    // 删除不存在的Key也会成功
    status = db->Delete(leveldb::WriteOptions(), key1);
    EXPECT_TRUE(status.ok());

    status = db->Delete(leveldb::WriteOptions(), key2);
    EXPECT_TRUE(status.ok());
}

TEST(LevelDbTest, SynchronousWrites)
{
    // 同步写，强制flush，但是效率会低很多，重要操作可以配合WriteBatch使用。
    // 在LevelDB中，所有的写入默认都是异步操作，所以写入函数会在写入磁盘缓存后就返回，并没有完全实际写入磁盘。
    // 数据真正写入磁盘的时间依赖于操作系统的操作。
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    leveldb::WriteOptions write_options;
    write_options.sync = true;

    string key = "key";

    static uint32_t COUNT = 50;
    CppShowTimer syncTimer;
    cout << syncTimer.Start("开始同步写") << endl;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        status = db->Put(write_options, key, "value");
        EXPECT_TRUE(status.ok());
    }
    cout << syncTimer.Record("同步写耗时") << endl;

    CppShowTimer asyncTimer;
    cout << asyncTimer.Start("开始异步写") << endl;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        status = db->Put(leveldb::WriteOptions(), key, "value");
        EXPECT_TRUE(status.ok());
    }
    cout << asyncTimer.Record("异步写耗时") << endl;

    /*
    [2016-01-23 14:57:18.259279]开始同步写
    [2016-01-23 14:57:18.363491]同步写耗时: From start 104212 us,from last 104212 us
    [2016-01-23 14:57:18.363567]开始异步写
    [2016-01-23 14:57:18.363892]异步写耗时: From start 325 us,from last 325 us
    50次同步写耗时是异步写的[320]倍.
    */
    cout << CppString::GetArgs("%u次同步写耗时是异步写的[%d]倍.", COUNT,
                               CppTime::TimevDiff(syncTimer.TimeRecorder.rbegin()->first, syncTimer.TimeRecorder.begin()->first) /
                               CppTime::TimevDiff(asyncTimer.TimeRecorder.rbegin()->first, asyncTimer.TimeRecorder.begin()->first))
        << endl;

    status = db->Delete(leveldb::WriteOptions(), key);
    EXPECT_TRUE(status.ok());
}

// 迭代器
// LevelDB中的迭代器可以用于遍历Key或者搜索Key
// 迭代方向可以正向或者逆向
// 迭代器使用完毕后要调用delete删除，可以使用unique_ptr删除
TEST(LevelDbTest, Iterator)
{
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    // 写入COUNT个数
    static uint32_t COUNT = 10;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 使用默认的写选项即可
        // levelDB的Key和Value的格式均为leveldb::Slice，它的构造方式和std::string很类似，可通过字符串或者string进行构造
        status = db->Put(leveldb::WriteOptions(), CppString::ToString(i), CppString::ToString(i));
        EXPECT_TRUE(status.ok());
    }

    // 遍历所有的值
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
    EXPECT_TRUE(it->status().ok());
    unique_ptr<leveldb::Iterator> uniqueIterator(it);
    uint32_t currKey = 0;
    for (it->SeekToFirst(); it->Valid(); it->Next(), ++currKey)
    {
        /*
        key[0]=0
        key[1]=1
        key[2]=2
        key[3]=3
        key[4]=4
        key[5]=5
        key[6]=6
        key[7]=7
        key[8]=8
        key[9]=9
        */
        // cout << CppString::GetArgs("key[%s]=%s", it->key().ToString().c_str(), it->value().ToString().c_str()) << endl;
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->key().ToString()));
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->value().ToString()));
        EXPECT_TRUE(it->status().ok());
    }

    // 按照字典序搜索
    currKey = 3;
    string endKey = "8";
    for (it->Seek(CppString::ToString(currKey)); it->Valid() && it->key().ToString() < endKey; it->Next(), ++currKey)
    {
        /*
        key[3]=3
        key[4]=4
        key[5]=5
        key[6]=6
        key[7]=7
        */
        // cout << CppString::GetArgs("key[%s]=%s", it->key().ToString().c_str(), it->value().ToString().c_str()) << endl;
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->key().ToString()));
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->value().ToString()));
        EXPECT_TRUE(it->status().ok());
    }
    EXPECT_EQ(endKey, it->key().ToString());

    // 逆向遍历
    currKey = 9;
    for (it->SeekToLast(); it->Valid(); it->Prev(), --currKey)
    {
        /*
        key[9]=9
        key[8]=8
        key[7]=7
        key[6]=6
        key[5]=5
        key[4]=4
        key[3]=3
        key[2]=2
        key[1]=1
        key[0]=0
        */
        // cout << CppString::GetArgs("key[%s]=%s", it->key().ToString().c_str(), it->value().ToString().c_str()) << endl;
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->key().ToString()));
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->value().ToString()));
        EXPECT_TRUE(it->status().ok());
    }

    // 删掉Key，可以使用迭代器删除全部的Key
    for (it->SeekToFirst(); it->Valid(); it->Next(), ++currKey)
    {
        db->Delete(leveldb::WriteOptions(), it->key());
        EXPECT_TRUE(it->status().ok());
    }
}

// 快照，可以保存一份当前DB完整的快照，保存快照之后对DB的操作不影响快照的内容
// 当快照使用完毕，需要调用DB::ReleaseSnapshot接口释放资源
TEST(LevelDbTest, Snapshots)
{
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    // 写入COUNT个数
    static uint32_t COUNT = 10;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 使用默认的写选项即可
        // levelDB的Key和Value的格式均为leveldb::Slice，它的构造方式和std::string很类似，可通过字符串或者string进行构造
        status = db->Put(leveldb::WriteOptions(), CppString::ToString(i), CppString::ToString(i));
        EXPECT_TRUE(status.ok());
    }

    leveldb::ReadOptions readOptions;
    readOptions.snapshot = db->GetSnapshot();
    leveldb::Iterator *it = db->NewIterator(readOptions);
    EXPECT_TRUE(it->status().ok());
    unique_ptr<leveldb::Iterator> uniqueIterator(it);

    // 删掉Key，可以使用迭代器删除全部的Key
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        db->Delete(leveldb::WriteOptions(), it->key());
        EXPECT_TRUE(it->status().ok());
    }

    // 遍历，快照迭代器，数据仍然存在
    uint32_t currKey = 0;
    it->SeekToFirst();
    EXPECT_TRUE(it->Valid());
    for (; it->Valid(); it->Next(), ++currKey)
    {
        /*
        key[0]=0
        key[1]=1
        key[2]=2
        key[3]=3
        key[4]=4
        key[5]=5
        key[6]=6
        key[7]=7
        key[8]=8
        key[9]=9
        */
        // cout << CppString::GetArgs("key[%s]=%s", it->key().ToString().c_str(), it->value().ToString().c_str()) << endl;
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->key().ToString()));
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->value().ToString()));
        EXPECT_TRUE(it->status().ok());
    }

    db->ReleaseSnapshot(readOptions.snapshot);

    // 遍历数据库，数据不存在了
    leveldb::Iterator *it2 = db->NewIterator(leveldb::ReadOptions());
    EXPECT_TRUE(it2->status().ok());
    unique_ptr<leveldb::Iterator> uniqueIterator2(it2);
    it2->SeekToFirst();
    EXPECT_FALSE(it2->Valid());
}

// 逆序比较器
// 继承于Comparator，必须实现4个父类中的纯虚函数，父类在<leveldb/comparator.h>中
class ReverseCompare : public leveldb::Comparator
{
public:

    int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const
    {
        const string &aStr = a.ToString();
        const string &bStr = b.ToString();
        if (bStr < aStr)
        {
            return -1;
        }

        if (aStr < bStr)
        {
            return 1;
        }

        return 0;
    }

    // 暂时忽略下面的函数实现:
    const char* Name() const
    {
        return "ReverseCompare";
    }

    void FindShortestSeparator(std::string*, const leveldb::Slice&) const
    {
    }

    void FindShortSuccessor(std::string*) const
    {
    }
};

// 比较器
// LevelDB支持自定义比较器，默认情况下，Key的比较使用字典序
// 同一个数据库使用不同的比较器作为参数打开，会出现兼容性问题，TODO，这里具体怎么解决还没尝试
TEST(LevelDbTest, Comparators)
{
    ReverseCompare reverseCompare;
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    options.comparator = &reverseCompare;

    // 不能打开之前的DB，每次打开之前，会检查比较器的名字，如果名字不匹配，则打开失败
    // leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    // EXPECT_TRUE(status.ok());
    // Invalid argument: leveldb.BytewiseComparator does not match existing comparator : ReverseCompare
    // cout << status.ToString() << endl;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTestComparators", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    // 写入COUNT个数
    static uint32_t COUNT = 10;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 使用默认的写选项即可
        // levelDB的Key和Value的格式均为leveldb::Slice，它的构造方式和std::string很类似，可通过字符串或者string进行构造
        status = db->Put(leveldb::WriteOptions(), CppString::ToString(i), CppString::ToString(i));
        EXPECT_TRUE(status.ok());
    }

    // 遍历出来，是逆序的
    int32_t currKey = COUNT - 1;
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
    unique_ptr<leveldb::Iterator> uniqueIt(it);
    it->SeekToFirst();
    EXPECT_TRUE(it->Valid());
    for (; it->Valid(); it->Next(), --currKey)
    {
        /*
        key[9]=9
        key[8]=8
        key[7]=7
        key[6]=6
        key[5]=5
        key[4]=4
        key[3]=3
        key[2]=2
        key[1]=1
        key[0]=0
        */
        //cout << CppString::GetArgs("key[%s]=%s", it->key().ToString().c_str(), it->value().ToString().c_str()) << endl;
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->key().ToString()));
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->value().ToString()));
        EXPECT_TRUE(it->status().ok());

        // 删除Key
        db->Delete(leveldb::WriteOptions(), it->key());
        EXPECT_TRUE(it->status().ok());
    }
}

// 性能调优
// 块大小(Block size)：这个是LevelDB存储的最小单位，默认大小是4096。
//      当应用读写的数据都是较大的数据时，可以适当提高这个值，反之应用读取的数据都是比较小的数据，可以适当降低这个值。
//      官方推荐尽量不要小于1K或者达到M级别。
//      同时需要注意的是，压缩功能在Block size比较大时，压缩效率更高。

// 压缩：LevelDB可以以块为单位进行数据压缩。当数据被写入磁盘前，会进行压缩，在读取数据时，会解压。
//      因为默认提供的压缩算法很快，所以压缩是默认开启的。
//      TODO：是否开启压缩，对于读取数据没有影响，只对写入的数据有影响。压缩后的数据，可以被未开启压缩的打开方式所读取。
TEST(LevelDbTest, Compression)
{
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());

    // 写入COUNT个数
    static uint32_t COUNT = 10;
    leveldb::WriteOptions write_options;
    write_options.sync = true;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        // 使用默认的写选项即可
        // levelDB的Key和Value的格式均为leveldb::Slice，它的构造方式和std::string很类似，可通过字符串或者string进行构造
        status = db->Put(write_options, CppString::ToString(i), CppString::ToString(i));
        EXPECT_TRUE(status.ok());
    }

    delete db;

    // 重新以不压缩的方式打开DB
    options.compression = leveldb::kNoCompression;
    status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueDb(db);

    // 读取数据，是可以读取出来的。
    // TODO：猜测是因为数据写入后有标记记录这个块是否被压缩
    int32_t currKey = 0;
    leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
    unique_ptr<leveldb::Iterator> uniqueIt(it);
    it->SeekToFirst();
    EXPECT_TRUE(it->Valid());
    for (; it->Valid(); it->Next(), ++currKey)
    {
        /*
        key[0]=0
        key[1]=1
        key[2]=2
        key[3]=3
        key[4]=4
        key[5]=5
        key[6]=6
        key[7]=7
        key[8]=8
        key[9]=9
        */
        // cout << CppString::GetArgs("key[%s]=%s", it->key().ToString().c_str(), it->value().ToString().c_str()) << endl;
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->key().ToString()));
        EXPECT_EQ(currKey, CppString::FromString<uint32_t>(it->value().ToString()));
        EXPECT_TRUE(it->status().ok());

        // 删除Key
        db->Delete(leveldb::WriteOptions(), it->key());
        EXPECT_TRUE(it->status().ok());
    }
}

// 缓存，LevelDB默认创建8M的缓存，也可以自己手工提供，但是需要释放，可以使用unique_ptr进行释放
// 缓存中保存的是未经过压缩的数据
TEST(LevelDbTest, Cache)
{
    // 提供16M的Cache
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    options.block_cache = leveldb::NewLRUCache(16 * 1024 * 1024);       // 16MB cache
    unique_ptr<leveldb::Cache> uniqueCache(options.block_cache);        // 注意需要释放资源

    leveldb::Status status = leveldb::DB::Open(options, "data/levelDbTest", &db);
    EXPECT_TRUE(status.ok());
    unique_ptr<leveldb::DB> uniqueIt(db);
}

// 过滤器：可以避免多次磁盘访问，比如NewBloomFilterPolicy可以用于随机读取比较大的应用中。
#endif
