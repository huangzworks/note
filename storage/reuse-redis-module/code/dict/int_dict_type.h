// int_dict_type.h

/*
 * key and value
 */
typedef struct {
    int value;
} KeyObject;

typedef struct {
    int value;
} ValueObject;

/*
 * prorotype
 */
KeyObject* create_key(void);
void release_key(KeyObject* key);

ValueObject* create_value(void);
void release_value(ValueObject* value);

/*
 * dict type
 */
unsigned int keyHashFunction(const void *key);
int keyCompareFunction(void *privdata, const void *x, const void *y);
void keyDestructor(void *privdata, void *key);
void valDestructor(void *privdata, void *value);
