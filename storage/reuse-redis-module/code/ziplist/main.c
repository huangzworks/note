// main.c

#include <assert.h> // load assert()
#include <stdlib.h> // load NULL

#include "ziplist.h"

void test_create_and_destroy_ziplist(void)
{
    unsigned char *zl = ziplistNew();

    assert(
        zl != NULL
    );

    zfree(zl);
}

void test_add_delete_and_find_node_wtih_ziplist(void)
{
    unsigned char *zl = ziplistNew();

    // add first node
    zl = ziplistPush(zl, (unsigned char*)"hello", 5, ZIPLIST_TAIL);

    assert(
        ziplistLen(zl) == 1
    );

    assert(
        ziplistFind(zl, (unsigned char*)"hello", 5, 0) != NULL)
    ;

    // add second node
    zl = ziplistPush(zl, (unsigned char*)"moto", 4, ZIPLIST_TAIL);

    assert(
        ziplistLen(zl) == 2
    );

    assert(
        ziplistFind(zl, (unsigned char*)"moto", 4, 0) != NULL);

    // delete first node
    unsigned char* node_p = ziplistFind(zl, (unsigned char*)"hello", 5, 0);
    zl = ziplistDelete(zl, &node_p);

    assert(
        ziplistFind(zl, (unsigned char*)"hello", 5, 0) == NULL
    );

    assert(
        ziplistLen(zl) == 1
    );

    // release memory
    zfree(zl);
}

void main(void)
{
    test_create_and_destroy_ziplist();

    test_add_delete_and_find_node_wtih_ziplist();
}
