// main.c

#include <stdlib.h> // 载入 NULL
#include <assert.h>

#include "intset.h"
#include "endianconv.h"

// Redis 将以下常量定义在了 intset.c
// 所以我们需要重新定义一次
// 如果将 intset.c 中的这些定义移动到 intset.h 
// 那么这里的定义就可以省去了
#define INTSET_ENC_INT16 (sizeof(int16_t))
#define INTSET_ENC_INT32 (sizeof(int32_t))
#define INTSET_ENC_INT64 (sizeof(int64_t))

void test_create_and_destroy_intset(void)
{
    intset *is = intsetNew();
    
    assert(
        is != NULL
    );

    assert(
        intsetLen(is) == 0
    );

    zfree(is);
}

void test_add_remove_and_find_element_in_intset(void)
{
    intset *is = intsetNew();

    assert(
        is != NULL
    );

    uint8_t success = 0;

    // add element
    is = intsetAdd(is, 1, &success);
    assert(
        success == 1 &&
        intsetLen(is) == 1
    );

    // add another element
    is = intsetAdd(is, 2, &success);
    assert(
        success == 1 &&
        intsetLen(is) == 2
    );

    // existsed element won't be add again
    is = intsetAdd(is, 2, &success);
    assert(
        success == 0 &&
        intsetLen(is) == 2
    );

    // remove 2 from is
    int int_success = 0;
    is = intsetRemove(is, 2, &int_success);
    assert(
        int_success == 1 &&
        intsetLen(is) == 1 &&
        intsetFind(is, 2) == 0
    );

    zfree(is);
}

void test_intset_upgrade(void)
{
    intset *is = intsetNew();

    is = intsetAdd(is, 32, NULL);
    assert(
        intrev32ifbe(is->encoding) == INTSET_ENC_INT16
    );

    is = intsetAdd(is, 65535, NULL);
    assert(
        intrev32ifbe(is->encoding) == INTSET_ENC_INT32
    );

    is = intsetAdd(is, 4294967295u, NULL);
    assert(
        intrev32ifbe(is->encoding) == INTSET_ENC_INT64
    );

    zfree(is);
}

void main(void)
{

    test_create_and_destroy_intset();

    test_add_remove_and_find_element_in_intset();

    test_intset_upgrade();

}
