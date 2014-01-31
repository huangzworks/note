// main.c

#include <assert.h>
#include <string.h>
#include "sds.h"

void create_sds_and_check_its_property(void)
{
    sds s = sdsnew("hello");

    // 验证长度
    assert(
        sdslen(s) == 5
    );

    // 验证空白位置
    assert(
        sdsavail(s) == 0
    );

    // 验证已有内容
    assert(
        memcmp(s, "hello\0", 6) == 0
    );

    // 释放
    sdsfree(s);
}


void test_sdsdup(void)
{
    sds s = sdsnew("hello");

    // 创建 s 的一个副本
    sds another = sdsdup(s);

    // 长度对比
    assert(
        sdslen(s) == sdslen(another)
    );

    // 空间对比
    assert(
        sdsavail(s) == sdsavail(another)
    );

    // 内容对比
    assert(
        memcmp(s, another, 6) == 0
    );

    // 释放
    sdsfree(s);
    sdsfree(another);
}


void test_sdscat(void)
{
    const char const total[] = "hello moto\0";

    sds s = sdsnew("hello");

    // 追加内容
    const char const append[] = " moto";
    sdscat(s, append);

    // 长度对比
    assert(
        sdslen(s) == strlen(total)
    );

    // 空间对比
    assert(
        // 追加之后的字符串
        // 会预留大小相当于现有字符串长度的空间
        sdsavail(s) == sdslen(s)
    );

    // 内容对比
    assert(
        memcmp(s, total, strlen(total)+1) == 0
    );

    // 释放空间
    sdsfree(s);
}


int main(void)
{
    create_sds_and_check_its_property();

    test_sdsdup();

    test_sdscat();
}
