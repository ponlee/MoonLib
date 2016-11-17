#include <iostream>
#include <sstream>
#include <functional>
#include <utility>
#include <string>

#include <boost/timer.hpp>
#include <boost/progress.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp> 
#include <boost/multi_index/member.hpp> 
#include <boost/multi_index/sequenced_index.hpp> 
#include <boost/multi_index/random_access_index.hpp> 
#include <boost/multi_index/member.hpp> 
#include <boost/multi_index/ordered_index.hpp> 
#include <boost/atomic.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>

#include <CppString.h>

#include "gtest/gtest.h"

using namespace boost;
using namespace std;
using namespace boost::interprocess;

TEST(Boost, timer)
{
    // 定义一个计时器，从定义的时刻开始计时，它大约只能支持几百小时的计数。
    timer t;

    cout << t.elapsed() << endl;            // 经过的时间，单位（秒）：1e-06
    cout << t.elapsed_max() << endl;        // 计量最大范围：9.22337e+12
    cout << t.elapsed_min() << endl;        // 计量最小范围：1e-06
}

TEST(Boost, progress_timer)
{
    progress_timer t;

    // 一些操作，在progress_timer析构后会自动向指定的输出流输出执行的时间，默认是std::cout

    /* 将结果输出到字符串流中 */
    stringstream ss;

    // 一个作用域，不然ss中不会有值
    {
        progress_timer t2(ss);
    }
    // cout << ss.str();
}

// 容器对象
struct Person
{
    std::string name;
    uint32_t age;
    bool male;

    Person(const std::string &n, uint32_t a, bool m = true)
        : name(n), age(a), male(m)
    {
    }
};

// 使用函数对象传入年龄
template <int age>
class SetAge
{
public:
    void operator()(Person &p)
    {
        p.age = age;
    }
};

// 多重索引容器，非序列接口
TEST(Boost, muilt_index)
{
    // 多重索引容器定义
    typedef boost::multi_index::multi_index_container <
        Person,             // 容器对象
        boost::multi_index::indexed_by <    // 指定可匹配的索引
        boost::multi_index::hashed_unique < boost::multi_index::member <Person, std::string, &Person::name> >,          // 索引0：姓名，要求唯一
        boost::multi_index::hashed_non_unique < boost::multi_index::member <Person, uint32_t, &Person::age> >,          // 索引1：年龄
        boost::multi_index::hashed_non_unique < boost::multi_index::member <Person, bool, &Person::male> >,             // 索引2：性别
        boost::multi_index::sequenced<>,                                                                                // 索引3：序列化接口
        boost::multi_index::random_access<>,                                                                            // 索引4：随机访问接口
        boost::multi_index::ordered_non_unique < boost::multi_index::member <Person, uint32_t, &Person::age> >          // 索引5：排序后的年龄
        >                                                                                                               // ...还可以增加更多的索引
    > person_multi;

    // 构造容器对象
    person_multi persons;

    // 插入元素
    persons.insert(Person("Boris", 31));
    persons.insert(Person("Boris", 32));                    // Key冲突，不会添加进去
    persons.insert(Person("Anton", 35, false));
    persons.insert(Person("Caesar", 25));

    // 默认为第一个索引接口：姓名
    EXPECT_EQ(1, persons.count("Boris"));

    // 获得年龄索引接口
    const person_multi::nth_index<1>::type &age_index = persons.get<1>();
    EXPECT_EQ(1, age_index.count(31));

    // 获得性别索引接口
    auto &male_index = persons.get<2>();
    EXPECT_EQ(2, male_index.count(true));

    // 修改年龄（使用函数对象）
    person_multi::iterator it = persons.find("Boris");
    persons.modify(it, SetAge<32>());
    EXPECT_EQ(1, age_index.count(32));

    // 修改年龄（使用lambda表达式）
    it = persons.find("Boris");
    persons.modify(it, [](Person &p)
    {
        p.age = 12;
    });
    EXPECT_EQ(1, age_index.count(12));

    // 序列化插入
    person_multi::nth_index<3>::type &sequence_index = persons.get<3>();
    sequence_index.push_back(Person("Moon", 26));

    // 获得年龄索引排序接口，注意这个不能和上面的排序、插入混用，否则会出现排序错误的情况，目前的例子就是
    person_multi::nth_index<5>::type &ordered_index = persons.get<5>();
    person_multi::nth_index<5>::type::iterator lower = ordered_index.lower_bound(30);   // 年龄大约30的第一个值
    person_multi::nth_index<5>::type::iterator upper = ordered_index.upper_bound(40);   // 年龄小于40的第一个值
    for (; lower != upper; ++lower)
    {
        cout << lower->name << endl;
    }

    // 随机访问接口
    const person_multi::nth_index<4>::type &random_access_index = persons.get<4>();
    EXPECT_EQ("Boris", random_access_index[0].name);
    EXPECT_EQ("Anton", random_access_index[1].name);
    EXPECT_EQ("Caesar", random_access_index[2].name);
}

// 原子变量操作封装
TEST(Boost, atomic)
{
    boost::atomic<int32_t> intAtomicValue(5);
    EXPECT_EQ(5, intAtomicValue);

    intAtomicValue = 0;
    EXPECT_EQ(0, intAtomicValue);

    ++intAtomicValue;
    EXPECT_EQ(1, intAtomicValue);

    int32_t oldValue = intAtomicValue.fetch_add(5);
    EXPECT_EQ(6, intAtomicValue);
    EXPECT_EQ(1, oldValue);
}

// 共享内存，文件名和偏移量handle确定唯一内存区域
TEST(Boost, InterprocessSharedMemory)
{
    // 共享内存名称，实际上是文件名
    static const char *MEMORY_NAME = "MySharedMemory";

    // 在构造和析构的时候负责删除共享内存，以便可以进行重复测试
    // Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
        ~shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
    } remover;

    // 创建共享内存
    // Create a managed shared memory segment
    static const managed_shared_memory::size_type MEMORY_SIZE = 65536;
    managed_shared_memory segment(create_only, MEMORY_NAME, MEMORY_SIZE);

    // 获得初始容量
    // Allocate a portion of the segment (raw memory)
    managed_shared_memory::size_type free_memory = segment.get_free_memory();
    EXPECT_LE(free_memory, MEMORY_SIZE);

    // 进行内存的分配，并且写入消息
    static const uint32_t MALLOC_SIZE = 1024;
    static const std::string CONTENT = "hello!";
    char *shptr = reinterpret_cast<char *>(segment.allocate(MALLOC_SIZE)); /* bytes to allocate */
    memcpy(shptr, CONTENT.c_str(), CONTENT.size() + 1);

    // 分配内存后，容量应该变小
    // Check invariant
    EXPECT_GT(free_memory, segment.get_free_memory());

    // 获得handle，这个表示内存地址在共享内存的位置，通过将这个位置传给其他进程，则可以访问到同一片内存区域
    // An handle from the base address can identify any byte of the shared
    // memory segment even if it is mapped in different base addresses
    managed_shared_memory::handle_t handle = segment.get_handle_from_address(shptr);

    /* 下面是模拟其他进程读取同一块共享内存 */
    // Open managed segment
    managed_shared_memory segmentChild(open_only, MEMORY_NAME);

    // 另一个进程获得内存的handle
    // An handle from the base address can identify any byte of the shared
    // memory segment even if it is mapped in different base addresses
    managed_shared_memory::handle_t handleChild = handle;

    // 读取消息，应该与写入的相同
    // Get buffer local address from handle
    char *msg = reinterpret_cast<char *>(segmentChild.get_address_from_handle(handleChild));
    EXPECT_EQ(CONTENT, msg);

    // 释放内存
    // Deallocate previously allocated memory
    segmentChild.deallocate(msg);

    // 检查子进程释放后内存是否恢复
    EXPECT_EQ(free_memory, segment.get_free_memory());
    EXPECT_EQ(segmentChild.get_free_memory(), segment.get_free_memory());
}

// 共享内存，文件名和偏移量handle确定唯一内存区域，在其中创建一个对象
TEST(Boost, InterprocessSharedMemoryWithObject)
{
    // 共享内存名称，实际上是文件名
    static const char *MEMORY_NAME = "MySharedMemory";

    typedef std::pair<double, int> MyType;

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
        ~shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
    } remover;

    //Construct managed shared memory
    managed_shared_memory segment(create_only, MEMORY_NAME, 65536);

    //Create an object of MyType initialized to {0.0, 0}
    MyType *instance = segment.construct < MyType >
        ("MyType instance")  //name of the object
        (0.0, 0);            //ctor first argument
    EXPECT_EQ(0, instance->second);

    //Create an array of 10 elements of MyType initialized to {0.0, 0}
    MyType *array = segment.construct < MyType >
        ("MyType array")     //name of the object
        [10]                 //number of elements
    (0.0, 123);            //Same two ctor arguments for all objects
    EXPECT_EQ(123, array[1].second);

    //Create an array of 3 elements of MyType initializing each one
    //to a different value {0.0, 0}, {1.0, 1}, {2.0, 2}...
    float float_initializer[3] = { 0.0, 1.0, 2.0 };
    int   int_initializer[3] = { 0, 1, 2 };

    MyType *array_it = segment.construct_it < MyType >
        ("MyType array from it")    // name of the object
        [3]                         // number of elements
    (&float_initializer[0]          // Iterator for the 1st ctor argument
     , &int_initializer[0]);        // Iterator for the 2nd ctor argument
    EXPECT_EQ(0, array_it[0].second);
    EXPECT_EQ(1, array_it[1].second);
    EXPECT_EQ(2, array_it[2].second);

    /* 下面是其他进程的操作 */
    // Open managed shared memory
    managed_shared_memory segmentChild(open_only, MEMORY_NAME);

    std::pair<MyType*, managed_shared_memory::size_type> res;       // <对象，对象的个数>

    // Find the array
    res = segmentChild.find<MyType>("MyType array");

    // Length should be 10
    EXPECT_EQ(10, res.second);
    EXPECT_EQ(123, res.first[0].second);

    // Find the object
    res = segmentChild.find<MyType>("MyType instance");

    // Length should be 1
    EXPECT_EQ(1, res.second);
    EXPECT_EQ(0, res.first[0].second);

    // Find the array constructed from iterators
    res = segmentChild.find<MyType>("MyType array from it");

    // Length should be 3
    EXPECT_EQ(3, res.second);
    EXPECT_EQ(0, res.first[0].second);
    EXPECT_EQ(1, res.first[1].second);
    EXPECT_EQ(2, res.first[2].second);

    // We're done, delete all the objects
    segmentChild.destroy<MyType>("MyType array");
    segmentChild.destroy<MyType>("MyType instance");
    segmentChild.destroy<MyType>("MyType array from it");

    // Check child has destroyed all objects
    EXPECT_FALSE(segment.find<MyType>("MyType array").first);
    EXPECT_FALSE(segment.find<MyType>("MyType instance").first);
    EXPECT_FALSE(segment.find<MyType>("MyType array from it").first);
}

// 共享内存，文件名和偏移量handle确定唯一内存区域，在其中创建一个偏移指针，指向当前共享内存中的值
TEST(Boost, InterprocessSharedMemoryWithPoint)
{
    // 共享内存名称，实际上是文件名
    static const char *MEMORY_NAME = "MySharedMemory";

    //Shared memory linked list node
    struct list_node
    {
        offset_ptr<list_node> next;
        int32_t               value;
    };

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
        ~shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
    } remover;

    //Create shared memory
    managed_shared_memory segment(create_only,
                                  MEMORY_NAME,  //segment name
                                  65536);

    //Create linked list with 10 nodes in shared memory
    offset_ptr<list_node> prev = 0, current, first;

    int32_t i;
    for (i = 0; i < 10; ++i, prev = current)
    {
        current = static_cast<list_node*>(segment.allocate(sizeof(list_node)));
        current->value = i;
        current->next = 0;

        if (!prev)
            first = current;
        else
            prev->next = current;
    }

    //Communicate list to other processes
    //. . .
    //When done, destroy list
    i = 0;
    for (current = first; current; /**/)
    {
        prev = current;
        EXPECT_EQ(i, current->value);
        ++i;

        current = current->next;
        segment.deallocate(prev.get());
    }
}

// 共享内存，文件名和偏移量handle确定唯一内存区域，在其中创建一个Vector
TEST(Boost, InterprocessSharedMemoryWithVector)
{
    // 共享内存名称，实际上是文件名
    static const char *MEMORY_NAME = "MySharedMemory";

    //Define an STL compatible allocator of ints that allocates from the managed_shared_memory.
    //This allocator will allow placing containers in the segment
    typedef boost::interprocess::allocator<int, managed_shared_memory::segment_manager>  ShmemAllocator;

    //Alias a vector that uses the previous STL-like allocator so that allocates
    //its values from the segment
    typedef boost::interprocess::vector<int, ShmemAllocator> MyVector;

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
        ~shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
    } remover;

    //Create a new segment with given name and size
    managed_shared_memory segment(create_only, MEMORY_NAME, 65536);

    //Initialize shared memory STL-compatible allocator
    const ShmemAllocator alloc_inst(segment.get_segment_manager());

    //Construct a vector named "MyVector" in shared memory with argument alloc_inst
    MyVector *myvector = segment.construct<MyVector>("MyVector")(alloc_inst);

    for (int i = 0; i < 100; ++i)  //Insert data in the vector
    {
        myvector->push_back(i);
    }

    /* 其他进程 */
    //Open the managed segment
    managed_shared_memory segmentChild(open_only, MEMORY_NAME);

    //Find the vector using the c-std::string name
    MyVector *myvectorChild = segmentChild.find<MyVector>("MyVector").first;

    //Use vector in reverse order
    std::sort(myvectorChild->rbegin(), myvectorChild->rend());

    int i = 99;
    for (MyVector::iterator it = myvector->begin(); it != myvector->end(); ++it)
    {
        EXPECT_EQ(i, *it);
        --i;
    }

    //When done, destroy the vector from the segment
    segmentChild.destroy<MyVector>("MyVector");

    //Check child has destroyed the vector
    EXPECT_FALSE(segment.find<MyVector>("MyVector").first);
}

// 共享内存，文件名和偏移量handle确定唯一内存区域，在其中创建一个Map
TEST(Boost, InterprocessSharedMemoryWithMap)
{
    // 共享内存名称，实际上是文件名
    static const char *MEMORY_NAME = "MySharedMemory";

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
        ~shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
    } remover;

    //Shared memory front-end that is able to construct objects
    //associated with a c-std::string. Erase previous shared memory with the name
    //to be used and create the memory segment at the specified address and initialize resources
    managed_shared_memory segment
        (create_only
        , MEMORY_NAME //segment name
        , 65536);          //segment size in bytes

    //Note that map<Key, MappedType>'s value_type is std::pair<const Key, MappedType>,
    //so the allocator must allocate that pair.
    typedef int32_t KeyType;
    typedef float   MappedType;
    typedef std::pair<const KeyType, MappedType> ValueType;

    //Alias an STL compatible allocator of for the map.
    //This allocator will allow to place containers
    //in managed shared memory segments
    typedef boost::interprocess::allocator<ValueType, managed_shared_memory::segment_manager> ShmemAllocator;

    //Alias a map of ints that uses the previous STL-like allocator.
    //Note that the third parameter argument is the ordering function
    //of the map, just like with std::map, used to compare the keys.
    typedef boost::interprocess::map<KeyType, MappedType, std::less<KeyType>, ShmemAllocator> MyMap;


    //Initialize the shared memory STL-compatible allocator
    ShmemAllocator alloc_inst(segment.get_segment_manager());
    MyMap mapInstance(std::less<KeyType>(), alloc_inst);

    //Construct a shared memory map.
    //Note that the first parameter is the comparison function,
    //and the second one the allocator.
    //This the same signature as std::map's constructor taking an allocator
    MyMap *mymap =
        segment.construct<MyMap>("MyMap")      //object name
        (std::less<KeyType>() //first  ctor parameter
        , alloc_inst);     //second ctor parameter

    //Insert data in the map
    for (KeyType i = 0; i < 100; ++i)
    {
        mymap->insert(std::pair<const KeyType, MappedType>(i, (MappedType)i));
    }

    for (KeyType i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i, (KeyType)(*mymap)[i]);
    }
}

// 文件映射
TEST(Boost, InterprocessMappedFiles)
{
    //Define file names
    static const char *FileName = "/tmp/file.bin";
    static const std::size_t FileSize = 10000;

    /* Parent process executes this */
    // Create a file
    {
        file_mapping::remove(FileName);
        std::filebuf fbuf;
        fbuf.open(FileName, std::ios_base::in | std::ios_base::out
                  | std::ios_base::trunc | std::ios_base::binary);
        //Set the size
        fbuf.pubseekoff(FileSize - 1, std::ios_base::beg);
        fbuf.sputc(0);

        // fbuf析构会自动关闭文件
    }

    //Remove on exit
    struct file_remove
    {
        file_remove(const char *FileName)
            : FileName_(FileName)
        {
        }
        ~file_remove()
        {
            file_mapping::remove(FileName_);
        }
        const char *FileName_;
    } remover(FileName);

    // Create a file mapping
    file_mapping m_file(FileName, read_write);

    // Map the whole file with read-write permissions in this process
    mapped_region region(m_file, read_write);

    //Get the address of the mapped region
    void * addr = region.get_address();
    EXPECT_NE(nullptr, addr);
    std::size_t size = region.get_size();
    EXPECT_EQ(FileSize, size);

    //Write all the memory to 1
    std::memset(addr, 1, size);

    /* 其他进程 */
    //Child process executes this
    //Open the file mapping and map it as read-only
    {
        file_mapping m_file_child(FileName, read_only);

        mapped_region region_child(m_file_child, read_only);

        //Get the address of the mapped region
        void * addr_child = region_child.get_address();
        std::size_t size_child = region_child.get_size();

        //Check that memory was initialized to 1
        const char *mem = static_cast<char*>(addr_child);
        for (std::size_t i = 0; i < size_child; ++i)
        {
            //Error checking memory
            EXPECT_EQ(1, *mem);
            ++mem;
        }
    }

    /* 文件测试 */
    //Now test it reading the file
    {
        std::filebuf fbuf_file;
        fbuf_file.open(FileName, std::ios_base::in | std::ios_base::binary);

        //Read it to memory
        std::vector<char> vect(FileSize, 0);
        fbuf_file.sgetn(&vect[0], std::streamsize(vect.size()));

        //Check that memory was initialized to 1
        const char *mem_file = static_cast<char*>(&vect[0]);
        for (std::size_t i = 0; i < FileSize; ++i)
        {
            //Error checking memory
            EXPECT_EQ(1, *mem_file);
            ++mem_file;
        }
    }
}

// 文件映射中放入一个Map
TEST(Boost, InterprocessMappedFilesWithMap)
{
    //Define file names
    static const char *FileName = "/tmp/file.bin";
    static const std::size_t FileSize = 10000;      // 如果空间不够，会在分配时抛出boost::interprocess::bad_alloc异常

    //Remove on exit
    file_mapping::remove(FileName);
    struct file_remove
    {
        file_remove(const char *FileName)
            : FileName_(FileName)
        {
        }
        ~file_remove()
        {
            file_mapping::remove(FileName_);
        }
        const char *FileName_;
    } remover(FileName);

    //Note that map<Key, MappedType>'s value_type is std::pair<const Key, MappedType>,
    //so the allocator must allocate that pair.
    typedef int32_t KeyType;
    typedef float   MappedType;
    typedef std::pair<const KeyType, MappedType> ValueType;

    //     typedef boost::interprocess::basic_managed_mapped_file <
    //         char,
    //         rbtree_best_fit<null_mutex_family>,
    //         iset_index
    //     > ManagedMapFile;

    // 定义分配器
    typedef boost::interprocess::allocator <
        ValueType,
        managed_mapped_file::segment_manager
    > MappedFileAllocator;

    // 定义容器
    //Alias a map of ints that uses the previous STL-like allocator.
    //Note that the third parameter argument is the ordering function
    //of the map, just like with std::map, used to compare the keys.
    typedef boost::interprocess::map<KeyType, MappedType, std::less<KeyType>, MappedFileAllocator> MyMap;

    // 创建映射文件
    managed_mapped_file mFile(create_only, FileName, FileSize);

    //Initialize the shared memory STL-compatible allocator
    MappedFileAllocator alloc_inst(mFile.get_segment_manager());

    //Construct a shared memory map.
    //Note that the first parameter is the comparison function,
    //and the second one the allocator.
    //This the same signature as std::map's constructor taking an allocator
    MyMap *mymap =
        mFile.construct<MyMap>("MyMap")      //object name
        (std::less<KeyType>() //first  ctor parameter
        , alloc_inst);     //second ctor parameter

    //Insert data in the map
    for (KeyType i = 0; i < 100; ++i)
    {
        mymap->insert(std::pair<const KeyType, MappedType>(i, (MappedType)i));
    }

    // 检查结果
    for (KeyType i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i, (KeyType)(*mymap)[i]);
    }

    /* 另一个实例读取 */
    managed_mapped_file mFileChild(open_only, FileName);
    MyMap *mymap_child = mFileChild.find<MyMap>("MyMap").first;

    // 检查结果
    for (KeyType i = 0; i < 100; ++i)
    {
        EXPECT_EQ(i, (KeyType)(*mymap_child)[i]);
    }
}

// 文件映射中放入一个unordered_map<uint64,string>
TEST(Boost, InterprocessMappedFilesWithUnorderedMap)
{
    //Define file names
    static const char *FileName = "/tmp/file.bin";
    static const std::size_t FileSize = 10000;      // 如果空间不够，会在分配时抛出boost::interprocess::bad_alloc异常

    //Remove on exit
    file_mapping::remove(FileName);
    struct file_remove
    {
        file_remove(const char *FileName)
            : FileName_(FileName)
        {
        }
        ~file_remove()
        {
            file_mapping::remove(FileName_);
        }
        const char *FileName_;
    } remover(FileName);

    typedef boost::interprocess::allocator <char, boost::interprocess::managed_mapped_file::segment_manager> MappedFileCharAllocator;                 // 内存分配器
    typedef boost::interprocess::basic_string<char, std::char_traits<char>, MappedFileCharAllocator> MappedFileString;               // 支持分配器的String
    typedef uint64_t KeyType;                                                                                       // Key的类型
    typedef MappedFileString ValueType;                                                                             // 值的类型

    typedef std::pair<const KeyType, ValueType> MapPairType;                                                        // map对的类型
    typedef boost::interprocess::allocator<MapPairType, boost::interprocess::managed_mapped_file::segment_manager> MappedFileAllocator;               // 分配器

    // 定义容器
    typedef boost::unordered_map<KeyType, ValueType, std::hash<KeyType>, std::equal_to<KeyType>, MappedFileAllocator > MyMap;

    // 创建映射文件
    managed_mapped_file mFile(create_only, FileName, FileSize);

    //Initialize the shared memory STL-compatible allocator
    MappedFileAllocator alloc_inst(mFile.get_segment_manager());
    MappedFileCharAllocator char_alloc_inst(mFile.get_segment_manager());

    //Construct a shared memory map.
    //Note that the first parameter is the comparison function,
    //and the second one the allocator.
    //This the same signature as std::map's constructor taking an allocator
    MyMap *mymap =
        mFile.construct<MyMap>("MyMap")      //object name
        (3, std::hash<KeyType>(), std::equal_to<KeyType>(), alloc_inst);

    //Insert data in the map
    for (KeyType i = 0; i < 100; ++i)
    {
        mymap->insert(MapPairType(i, MappedFileString((boost::lexical_cast<std::string>(i)).c_str(), char_alloc_inst)));
    }

    // 检查结果
    for (KeyType i = 0; i < 100; ++i)
    {
        MappedFileString v((boost::lexical_cast<std::string>(i)).c_str(), char_alloc_inst);
        EXPECT_EQ(0, mymap->find(i)->second.compare(v));
    }

    /* 另一个实例读取 */
    managed_mapped_file mFileChild(open_only, FileName);
    MyMap *mymap_child = mFileChild.find<MyMap>("MyMap").first;

    // 检查结果
    for (KeyType i = 0; i < 100; ++i)
    {
        MappedFileString v((boost::lexical_cast<std::string>(i)).c_str(), char_alloc_inst);
        EXPECT_EQ(0, mymap_child->find(i)->second.compare(v));
    }
}

// 文件映射中放入一个MuiltyIndex
TEST(Boost, InterprocessMappedFilesWithMuiltyIndex)
{
    //Define file names
    static const char *FileName = "/tmp/file.bin";
    static const std::size_t FileSize = 10000;      // 如果空间不够，会在分配时抛出boost::interprocess::bad_alloc异常

    //Remove on exit
    file_mapping::remove(FileName);
    struct file_remove
    {
        file_remove(const char *FileName)
            : FileName_(FileName)
        {
        }
        ~file_remove()
        {
            file_mapping::remove(FileName_);
        }
        const char *FileName_;
    } remover(FileName);

    // 定义分配器
    typedef boost::interprocess::allocator <Person, managed_mapped_file::segment_manager> MappedFileAllocator;

    // 多重索引容器定义
    typedef boost::multi_index::multi_index_container <
        Person,             // 容器对象
        boost::multi_index::indexed_by <    // 指定可匹配的索引
        boost::multi_index::hashed_unique < boost::multi_index::member <Person, std::string, &Person::name> >,     // 索引0：姓名，要求唯一
        boost::multi_index::hashed_non_unique < boost::multi_index::member <Person, uint32_t, &Person::age> >,          // 索引1：年龄
        boost::multi_index::hashed_non_unique < BOOST_MULTI_INDEX_MEMBER(Person, bool, male) >,                         // 索引2：性别
        boost::multi_index::sequenced<>,                                                                                // 索引3：序列化接口
        boost::multi_index::random_access<>,                                                                            // 索引4：随机访问接口
        boost::multi_index::ordered_non_unique < boost::multi_index::member <Person, uint32_t, &Person::age> >          // 索引5：排序后的年龄
        > ,                                                                                                             // ...还可以增加更多的索引
        MappedFileAllocator                // 分配器
    > person_multi;

    // 创建映射文件
    managed_mapped_file mFile(open_or_create, FileName, FileSize);

    //Initialize the shared memory STL-compatible allocator
    MappedFileAllocator alloc_inst(mFile.get_segment_manager());

    // 构造容器对象
    person_multi *pPersons = mFile.construct<person_multi>("Person")(alloc_inst);
    person_multi &persons = *pPersons;

    // 插入元素
    persons.insert(Person("Boris", 31));
    persons.insert(Person("Boris", 32));                    // Key冲突，不会添加进去
    persons.insert(Person("Anton", 35, false));
    persons.insert(Person("Caesar", 25));

    // 默认为第一个索引接口：姓名
    EXPECT_EQ(1, persons.count("Boris"));

    // 获得年龄索引接口
    const person_multi::nth_index<1>::type &age_index = persons.get<1>();
    EXPECT_EQ(1, age_index.count(31));

    // 获得性别索引接口
    auto &male_index = persons.get<2>();
    EXPECT_EQ(2, male_index.count(true));

    // 修改年龄（使用函数对象）
    person_multi::iterator it = persons.find("Boris");
    persons.modify(it, SetAge<32>());
    EXPECT_EQ(1, age_index.count(32));

    // 修改年龄（使用lambda表达式）
    it = persons.find("Boris");
    persons.modify(it, [](Person &p)
    {
        p.age = 12;
    });
    EXPECT_EQ(1, age_index.count(12));

    // 序列化插入
    person_multi::nth_index<3>::type &sequence_index = persons.get<3>();
    sequence_index.push_back(Person("Moon", 26));

    // 获得年龄索引排序接口，注意这个不能和上面的排序、插入混用，否则会出现排序错误的情况，目前的例子就是
    person_multi::nth_index<5>::type &ordered_index = persons.get<5>();
    person_multi::nth_index<5>::type::iterator lower = ordered_index.lower_bound(30);   // 年龄大约30的第一个值
    person_multi::nth_index<5>::type::iterator upper = ordered_index.upper_bound(40);   // 年龄小于40的第一个值
    for (; lower != upper; ++lower)
    {
        cout << lower->name << endl;
    }

    // 随机访问接口
    const person_multi::nth_index<4>::type &random_access_index = persons.get<4>();
    EXPECT_EQ("Boris", random_access_index[0].name);
    EXPECT_EQ("Anton", random_access_index[1].name);
    EXPECT_EQ("Caesar", random_access_index[2].name);

    /* 另一个实例读取 */
    managed_mapped_file mFileChild(open_only, FileName);
    person_multi *pPersonChild = mFileChild.find<person_multi>("Person").first;
    person_multi &personsChild = *pPersonChild;

    // 默认为第一个索引接口：姓名
    EXPECT_EQ(1, personsChild.count("Boris"));

    // 获得年龄索引接口
    const person_multi::nth_index<1>::type &age_index_child = personsChild.get<1>();
    EXPECT_EQ(1, age_index_child.count(12));

    // 获得性别索引接口
    auto &male_index_child = personsChild.get<2>();
    EXPECT_EQ(3, male_index_child.count(true));

    // 修改年龄（使用函数对象）
    person_multi::iterator it_child = personsChild.find("Boris");
    personsChild.modify(it_child, SetAge<32>());
    EXPECT_EQ(1, age_index_child.count(32));

    // 修改年龄（使用lambda表达式）
    it_child = personsChild.find("Boris");
    personsChild.modify(it_child, [](Person &p)
    {
        p.age = 12;
    });
    EXPECT_EQ(1, age_index_child.count(12));

    // 随机访问接口
    const person_multi::nth_index<4>::type &random_access_index_child = personsChild.get<4>();
    EXPECT_EQ("Boris", random_access_index_child[0].name);
    EXPECT_EQ("Anton", random_access_index_child[1].name);
    EXPECT_EQ("Caesar", random_access_index_child[2].name);
}

// 文件映射中放入一个MuiltyIndex，里面含有容器
TEST(Boost, InterprocessMappedFilesWithMuiltyIndex2)
{
    typedef boost::interprocess::allocator <char, boost::interprocess::managed_mapped_file::segment_manager> MappedFileCharAllocator;                    // 内存分配器
    typedef boost::interprocess::basic_string<char, std::char_traits<char>, MappedFileCharAllocator> MappedFileString;

    // 容器对象
    struct Person2
    {
        MappedFileString name;
        uint32_t age;
        bool male;

        Person2(MappedFileCharAllocator &acclocator, const std::string &n, uint32_t a, bool m = true)
            :name(acclocator), age(a), male(m)
        {
            name.assign(n.c_str());
        }
    };

    struct MappedFileStringHash
    {
        uint32_t operator()(const MappedFileString &p1)
        {
            return std::hash<std::string>()(p1.c_str());
        }
    };

    struct MappedFileStringEquel
    {
        bool operator()(const MappedFileString &p1, const MappedFileString &p2)
        {
            return p1 == p2;
        }
    };

    //Define file names
    static const char *FileName = "/tmp/file2.bin";
    static const std::size_t FileSize = 10000;      // 如果空间不够，会在分配时抛出boost::interprocess::bad_alloc异常

    bool fileExist = (boost::filesystem::status(FileName).type() == boost::filesystem::regular_file);     // 判断文件是否存在

    // 定义分配器
    typedef boost::interprocess::allocator <Person2, managed_mapped_file::segment_manager> MappedFileAllocator;

    // 多重索引容器定义
    typedef boost::multi_index::multi_index_container <
        Person2,             // 容器对象
        boost::multi_index::indexed_by <    // 指定可匹配的索引
        boost::multi_index::hashed_unique < BOOST_MULTI_INDEX_MEMBER(Person2, MappedFileString, name) >,            // 索引0：姓名，要求唯一
        boost::multi_index::hashed_unique < BOOST_MULTI_INDEX_MEMBER(Person2, uint32_t, age) >                      // 索引0：姓名，要求唯一
        >,                                                                                                         // ...还可以增加更多的索引
        // MappedFileAllocator                // 分配器
        managed_mapped_file::segment_manager::allocator<Person2>::type          // 也可以用这样的方式定义分配器类型
    > person_multi;

    // 创建映射文件
    managed_mapped_file mFile(open_or_create, FileName, FileSize);

    //Initialize the shared memory STL-compatible allocator
    MappedFileAllocator alloc_inst(mFile.get_segment_manager());
    MappedFileCharAllocator alloc_char_inst(mFile.get_segment_manager());

    // 构造容器对象
    person_multi *pPersons;
    static const char *MEM_KEY = "Person";

    if (!fileExist)
    {
        // 文件不存在，则创建对象
        cout << CppString::GetArgs("%s not exists!", FileName) << endl;

        // pPersons = mFile.construct<person_multi>(MEM_KEY)(alloc_inst);
        pPersons = mFile.construct<person_multi>(MEM_KEY)(person_multi::ctor_args_list(), alloc_inst);       // 还可以使用ctor_args_list代表默认构造函数？
        pPersons->insert(Person2(alloc_char_inst, "hello", 12));
        mFile.flush();
    }
    else
    {
        // 文件已存在，则获得对象
        cout << CppString::GetArgs("%s exists!", FileName) << endl;

        pPersons = mFile.find<person_multi>(MEM_KEY).first;
        EXPECT_NE(static_cast<void *>(NULL), pPersons);

        MappedFileString findStr(alloc_char_inst);
        findStr.assign("hello");

        person_multi::iterator findResult = pPersons->find(findStr);
        EXPECT_NE(pPersons->end(), findResult);
        EXPECT_EQ(12, findResult->age);
        EXPECT_EQ(true, findResult->male);

        //Remove on exit
        struct file_remove
        {
            file_remove(const char *FileName)
                : FileName_(FileName)
            {
            }

            ~file_remove()
            {
                file_mapping::remove(FileName_);
            }

            const char *FileName_;
        } remover(FileName);
    }
}

// Boost的string容器测试
TEST(Boost, shrink_to_fit)
{
    // 共享内存名称，实际上是文件名
    static const char *MEMORY_NAME = "MySharedMemory";

    typedef boost::interprocess::allocator <char, boost::interprocess::managed_shared_memory::segment_manager> SharedCharAllocator;                    // 内存分配器
    typedef boost::interprocess::basic_string<char, std::char_traits<char>, SharedCharAllocator> MappedFileString;

    //Remove shared memory on construction and destruction
    struct shm_remove
    {
        shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
        ~shm_remove()
        {
            shared_memory_object::remove(MEMORY_NAME);
        }
    } remover;

    //Create a new segment with given name and size
    managed_shared_memory segment(create_only, MEMORY_NAME, 65536);

    //Initialize shared memory STL-compatible allocator
    const SharedCharAllocator alloc_inst(segment.get_segment_manager());

    MappedFileString str(alloc_inst);
    str.assign("12312312312312312123123");
    cout << "begin: " << str.size() << " / " << str.capacity() << endl;

    for (uint32_t i = 0; i < 100; ++i)
    {
        str.append("12312312312312312123123");
    }

    cout << "after append: " << str.size() << " / " << str.capacity() << endl;

    str = "123";

    cout << "after assign: " << str.size() << " / " << str.capacity() << endl;

    str.shrink_to_fit();
    cout << "after shrink_to_fit: " << str.size() << " / " << str.capacity() << endl;

    for (uint32_t i = 0; i < 100; ++i)
    {
        str.append("12312312312312312123123");
    }

    cout << "after append: " << str.size() << " / " << str.capacity() << endl;

    MappedFileString str2("123", alloc_inst);
    str.swap(str2);
    cout << "after swap: " << str.size() << " / " << str.capacity() << endl;
}

TEST(Boost, optional)
{
    {
        int32_t x = 10;
        optional<int32_t> optional_value(x != 0, 30.0 / x);        // 有值=3
        optional<int32_t> optional_value_none(x == 0, 1);          // 无值

        if (!optional_value)
        {
            // 不会进入
            EXPECT_TRUE(false);
        }

        // EXPECT_TRUE(optional_value == true);         // 编译不过
        // EXPECT_EQ(3, optional_value);                // 编译不过
        EXPECT_TRUE(3 == optional_value);               // 可直接与容器内的类型值比较，但是如果容器内是uint32_t，则不能直接与int32_t类型比较，需要强转
        EXPECT_TRUE(optional_value == 3);
        EXPECT_EQ(3, optional_value.get());
        EXPECT_EQ(3, optional_value.get_value_or(1));

        if (optional_value_none)
        {
            // 不会进入
            EXPECT_TRUE(false);
        }

        // EXPECT_FALSE(optional_value_none);           // 编译不过
        // EXPECT_FALSE(optional_value_none == true);   // 编译不过
        EXPECT_FALSE(3 == optional_value_none);
        EXPECT_FALSE(optional_value_none == 3);
        // EXPECT_EQ(3, optional_value_none.get());     // 无值，不能使用get，会assert掉。
        EXPECT_EQ(1, optional_value_none.get_value_or(1));
    }

    {
        int32_t x = 0;
        optional<int32_t> optional_value(x == 0, 3);                // 有值=3
        optional<int32_t> optional_value_none(x != 0, 30.0 / x);    // 无值，-O2的话并且不会执行30.0/x而core掉，-O1和-O0会编译报错

        if (!optional_value)
        {
            // 不会进入
            EXPECT_TRUE(false);
        }

        // EXPECT_TRUE(optional_value);                 // 编译不过
        // EXPECT_TRUE(optional_value == true);         // 编译不过
        // EXPECT_EQ(3, optional_value);                // 编译不过
        EXPECT_TRUE(3 == optional_value);
        EXPECT_TRUE(optional_value == 3);
        EXPECT_EQ(3, optional_value.get());
        EXPECT_EQ(3, optional_value.get_value_or(1));

        if (optional_value_none)
        {
            // 不会进入
            EXPECT_TRUE(false);
        }

        // EXPECT_FALSE(optional_value_none == true);   // 编译不过
        EXPECT_FALSE(3 == optional_value_none);
        EXPECT_FALSE(optional_value_none == 3);
        // EXPECT_EQ(3, optional_value_none.get());     // 无值，不能使用get，会assert掉。
        EXPECT_EQ(1, optional_value_none.get_value_or(1));
    }
}

TEST(Boost, lexical_cast)
{
    EXPECT_EQ("123", boost::lexical_cast<std::string>(123));
    EXPECT_EQ(123, boost::lexical_cast<uint32_t>("123"));
    EXPECT_EQ(-123, boost::lexical_cast<int32_t>("-123"));
    EXPECT_EQ(-1, boost::lexical_cast<int32_t>("-123", 2));
    EXPECT_THROW(boost::lexical_cast<int32_t>("123abc"), boost::bad_lexical_cast);        // 抛出异常
}
