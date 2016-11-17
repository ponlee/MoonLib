#include <iostream>

#include <stdint.h>
#include "gtest/gtest.h"

using namespace std;

void print_arr(int *arr, int n)
{
    int i;
    printf("%d", arr[0]);
    for (i = 1; i < n; i++)
        printf(" %2d", arr[i]);
    printf("\n");
}

void counting_sort(int *ini_arr, int *sorted_arr, int n)
{
    int *count_arr = (int *)malloc(sizeof(int) * 100);
    int i, j, k;

    // 初始化计数数组
    for (k = 0; k < 100; k++)
        count_arr[k] = 0;
    print_arr(count_arr, 100);          // 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0

    // 对每个输入的数字，使其对应的计数值加1
    for (i = 0; i < n; i++)
        count_arr[ini_arr[i]]++;
    print_arr(count_arr, 100);          // 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0  1  0  0  0  0  0  0  0  1  2  0  0  0  0  0  1  0  1  0  0  0  0  0  1  0  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0

    // 计算每个索引位置对应的排序位置
    for (k = 1; k < 100; k++)
        count_arr[k] += count_arr[k - 1];
    print_arr(count_arr, 100);          // 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  1  1  2  2  2  2  2  2  2  2  3  5  5  5  5  5  5  6  6  7  7  7  7  7  7  8  8  8  8  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9  9 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10

    // 核心部分：对每个输入的数字，获得其排序位置，为了处理有多个数值相同，需要对已经取出数值的位置减去1，从而计算下一个数字的正确索引
    for (j = n; j > 0; j--)
    {
        /*
        j=10,ini_arr[j - 1]=28,--count_arr[ini_arr[j - 1]]= 2
        j= 9,ini_arr[j - 1]=47,--count_arr[ini_arr[j - 1]]= 8
        j= 8,ini_arr[j - 1]=29,--count_arr[ini_arr[j - 1]]= 4
        j= 7,ini_arr[j - 1]=20,--count_arr[ini_arr[j - 1]]= 1
        j= 6,ini_arr[j - 1]=17,--count_arr[ini_arr[j - 1]]= 0
        j= 5,ini_arr[j - 1]=29,--count_arr[ini_arr[j - 1]]= 3
        j= 4,ini_arr[j - 1]=43,--count_arr[ini_arr[j - 1]]= 7
        j= 3,ini_arr[j - 1]=35,--count_arr[ini_arr[j - 1]]= 5
        j= 2,ini_arr[j - 1]=37,--count_arr[ini_arr[j - 1]]= 6
        j= 1,ini_arr[j - 1]=85,--count_arr[ini_arr[j - 1]]= 9
        */
        printf("j=%2d,ini_arr[j - 1]=%2d,--count_arr[ini_arr[j - 1]]=%2d\n", j, ini_arr[j - 1], count_arr[ini_arr[j - 1]] - 1);
        sorted_arr[--count_arr[ini_arr[j - 1]]] = ini_arr[j - 1];
    }
    free(count_arr);
}

// 计数排序
TEST(SortTest, Count)
{
    int n = 10;
    int i;
    int *arr = (int *)malloc(sizeof(int) * n);
    int *sorted_arr = (int *)malloc(sizeof(int) * n);
    srand(time(0));
    for (i = 0; i < n; i++)
        arr[i] = rand() % 100;
    printf("ini_array: ");      // ini_array: 85 37 35 43 29 17 20 29 47 28
    print_arr(arr, n);
    counting_sort(arr, sorted_arr, n);
    printf("sorted_array: ");   // sorted_array: 17 20 28 29 29 35 37 43 47 85
    print_arr(sorted_arr, n);
    free(arr);
    free(sorted_arr);
}

void QuickSort(vector<int32_t> &array, int32_t l, int32_t u)
{
    if (l >= u)
    {
        return;
    }

    // 这个交换是为了每次循环更加容易分成2个均等的子数组
    swap(array[l], array[(l + u) / 2]);

    uint32_t m = l;
    for (uint32_t i = l + 1; i <= u; ++i)
    {
        if (array[i] < array[l])
        {
            swap(array[++m], array[i]);
        }
    }

    swap(array[l], array[m]);
    QuickSort(array, l, m - 1);
    QuickSort(array, m + 1, u);
}

void PrintVec(vector<int32_t> &array)
{
    for (auto &value : array)
    {
        printf("%2d ", value);
    }

    printf("\n");
}

// 快速排序
TEST(SortTest, QuickSort)
{
    vector<int32_t> array{4, 5, 6, 7, 1, 2, 3, 8, 9};
    PrintVec(array);
    QuickSort(array, 0, array.size() - 1);
    PrintVec(array);
}
