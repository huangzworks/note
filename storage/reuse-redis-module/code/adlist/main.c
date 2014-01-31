// main.c

#include <assert.h>
#include <stdlib.h>

#include "adlist.h"
#include "object.h"

void test_empty_list(void)
{
    // 创建一个新链表
    list* l = listCreate();

    // 检查表头和表尾
    assert(
        l->head == NULL &&
        l->tail == NULL
    );

    // 检查节点数量
    assert(
        l->len == 0
    );

    // 检查类型限定函数
    assert(
        l->dup == NULL &&
        l->free == NULL &&
        l->match == NULL
    );

    // 释放链表
    listRelease(l);
}


void test_add_node_and_advance_by_pointer(void)
{

    // 初始化值对象
    object *x = create_object(),
           *y = create_object(),
           *z = create_object();


    // 创建列表
    list* l = listCreate();
    // l = [x]
    listAddNodeHead(l, x);
    // l = [x, z]
    listAddNodeTail(l, z);
    // l = [x, y, z]
    listNode* node_contain_x = listSearchKey(l, x);
    listInsertNode(l, node_contain_x, y, 1);   //insert y after x


    // 手动遍历节点
    listNode* current;

    // x
    current = listFirst(l);
    assert(
        current->value == x
    );

    // y
    current = listNextNode(current);
    assert(
        current->value == y
    );

    // z
    current = listNextNode(current);
    assert(
        current->value == z
    );

    
    // 释放空间
    free_object(x);
    free_object(y);
    free_object(z);

    listRelease(l);
}


void test_iterator(void)
{
    // 初始化值对象
    object *x = create_object(),
           *y = create_object(),
           *z = create_object();


    // 创建列表
    list* l = listCreate();
    listAddNodeTail(l, x);
    listAddNodeTail(l, y);
    listAddNodeTail(l, z);


    // 从表头向表尾遍历节点
    listIter* itertor = listGetIterator(l, AL_START_HEAD);
    listNode* current;
    
    // 第一个节点
    current = listNext(itertor);
    assert(
        current->value == x
    );

    // 第二个节点
    current = listNext(itertor);
    assert(
        current->value == y
    );

    // 第三个节点
    current = listNext(itertor);
    assert(
        current->value == z
    );


    // 释放
    free_object(x);
    free_object(y);
    free_object(z);

    listRelease(l);

    listReleaseIterator(itertor);
}

void main(void)
{
    test_empty_list();

    test_add_node_and_advance_by_pointer();

    test_iterator();
}
