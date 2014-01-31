#include <stdlib.h>
#include <assert.h>

#include "int_dict_type.h"
#include "dict.h"

extern dictType intDictType;

void test_empty_dict(void)
{
    dict* d = dictCreate(&intDictType, NULL);

    dictRelease(d);
}

void test_add_and_delete_key_value_pair(void)
{
    // 创建新字典
    dict *d = dictCreate(&intDictType, NULL);

    // 创建键和值
    KeyObject *k = create_key();
    k->value = 1;
    ValueObject *v = create_value();
    v->value = 10086;

    // 添加键值对
    dictAdd(d, k, v);

    assert(
        dictSize(d) == 1
    );

    assert(
        dictFind(d, k) != NULL
    );

    // 删除键值对
    dictDelete(d, k);

    assert(
        dictSize(d) == 0
    );

    assert(
        dictFind(d, k) == NULL
    );

    // 释放字典
    dictRelease(d);
}

void main(void)
{

    test_empty_dict();

    test_add_and_delete_key_value_pair();

}
