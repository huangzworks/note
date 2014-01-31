// object.c

#include "zmalloc.h"
#include "object.h"

object* create_object(void)
{
    return (object*)zmalloc(sizeof(object));
}

void free_object(object* obj)
{
    zfree(obj);
}
