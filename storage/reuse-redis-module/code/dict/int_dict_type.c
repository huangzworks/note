// int_dict_type.c

#include <stdlib.h> // NULL

#include "zmalloc.h"
#include "dict.h"
#include "int_dict_type.h"


/*
 * key
 */
KeyObject* create_key(void)
{
    return (KeyObject*)zmalloc(sizeof(KeyObject));
}

void release_key(KeyObject* key)
{
    zfree(key);
}


/*
 * value
 */
ValueObject* create_value(void)
{
    return (ValueObject*)zmalloc(sizeof(ValueObject));
}

void release_value(ValueObject* value)
{
    zfree(value);
}


/* 
 * dict type
 */
unsigned int keyHashFunction(const void *key)
{
    KeyObject *k = (KeyObject*)key;

    int value = k->value;

    if (value < 0)
        return 0 - value;
    else
        return value;
}

int keyCompareFunction(void *privdata, const void *x, const void *y)
{
    DICT_NOTUSED(privdata);

    KeyObject *kx = (KeyObject*)x;
    KeyObject *ky = (KeyObject*)y;

    return (kx->value == ky->value);
}

void keyDestructor(void *privdata, void *key)
{
    DICT_NOTUSED(privdata);

    release_key(key);
}

void valDestructor(void *privdata, void *value)
{
    DICT_NOTUSED(privdata);

    release_value(value);
}

dictType intDictType = {
    keyHashFunction,
    NULL,
    NULL,
    keyCompareFunction,
    keyDestructor,
    valDestructor
};
