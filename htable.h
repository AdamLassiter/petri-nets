#pragma once

#include "llist.h"

#include <stdlib.h>
#include <string.h>

#define HTABLE_DELETED (HTableEntry *) -1


typedef unsigned long long htable_int64;

extern const float htable_threshold;

typedef htable_int64 (*hash_func)(void *);

typedef struct HTableEntry {
    void *key;
    void *value;
} HTableEntry;

typedef struct HTable {
    HTableEntry **table;
    hash_func hash;
    htable_int64 table_size;
    htable_int64 size;
} HTable;


htable_int64 htable_quickhash64(void *);

HTable *htable_new(htable_int64, hash_func);
void htable_free(HTable *);

HTable *htable_from_llist(LList *, hash_func);
LList *htable_to_llist(HTable *);

void htable_put(HTable *, void *, void *);
void *htable_get(HTable *, void *);

HTable *htable_intersect(HTable *, HTable *);
HTable *htable_union(HTable *, HTable *);

void htable_resize(HTable *, htable_int64);
void htable_remove(HTable *, void *);
