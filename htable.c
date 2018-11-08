#include "htable.h"

#define min(a,b) \
    ({__typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })

#define max(a,b) \
    ({__typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })


const float htable_threshold = .75;


// Get the n'th byte of a given value
char byteof(unsigned long long value, int n) {
    return (value >> (8*n)) & 0xff;
}

// Quich 64-bit hash on void pointer addresses
htable_int64 htable_quickhash64(void *value) {
    const htable_int64 mulp = 2654435789;
    htable_int64 mix = 104395301;
    for (int i = 0; i < 8; i++)
        mix += (byteof((htable_int64) value, i) * mulp) ^ (mix >> 23);
    return mix ^ (mix << 37);
}


// Create a new dynamically-allocated hash table
HTable *htable_new(htable_int64 table_size, hash_func hash) {
    HTable *h = (HTable *) malloc(sizeof(*h));
    *h = (HTable) {
        .table = (HTableEntry **) calloc(sizeof(*h->table), table_size),
        .hash = hash,
        .table_size = table_size,
        .size = 0
    };
    return h;
}

// Free a dynamically-allocated hash table
void htable_free(HTable *h) {
    for (htable_int64 i = 0; i < h->table_size; i++)
        if (h->table[i] != NULL && h->table[i] != HTABLE_DELETED)
            free(h->table[i]);
    free(h->table);
    free(h);
}

// Create an entry into the table for a given key/value pair
HTableEntry *htable_entry_new(void *key, void *value) {
    HTableEntry *e = (HTableEntry *) malloc(sizeof(*e));
    *e = (HTableEntry) {
        .key = key,
        .value = value
    };
    return e;
}


// Convert a dense llist to a set-like sparse htable ( {a:a, b:b, c:c etc})
HTable *htable_from_llist(LList *l, hash_func hash) {
    HTable *h = htable_new(l->len * 2, hash);
    for (LListNode *ll_item = l->head; ll_item != NULL; ll_item = ll_item->next)
        htable_put(h, ll_item->element, ll_item->element);
    return h;
}

// Convert a sparse htable to a dense llist of keys
LList *htable_to_llist(HTable *h) {
    LList *l = llist_new();
    HTableEntry *entry;
    for (htable_int64 i = 0; i < h->table_size; i++)
        if ((entry = h->table[i]) != NULL &&
                entry != HTABLE_DELETED)
            llist_append(l, entry->key);
    return l;
}


/* ~~~ Here ~ Be ~ Dragons ~~~
 *                  \||/
 *                 |  @___oo
 *       /\  /\   / (__,,,,|
 *      ) /^\) ^\/ _)
 *      )   /^\/   _)
 *      )   _ /  / _)
 *  /\  )/\/ ||  | )_)
 * <  >      |(,,) )__)
 *  ||      /    \)___)\
 *  | \____(      )___) )___
 *   \______(_______;;; __;;;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// Add an element to a hashtable
void htable_put(HTable *h, void *key, void *value) {
    htable_int64 hash = h->hash(key) % h->table_size;
    // Hash collision - existing key/value pairs for different keys
    while (h->table[hash] != NULL &&
            h->table[hash] != HTABLE_DELETED &&
            h->table[hash]->key != key) {
        // Step forward until we hit a space 
        hash = (hash + 1) % h->table_size;
    }
    // If a space is found, but also a deleted space was stepped over
    if (h->table[hash] == NULL || h->table[hash] == HTABLE_DELETED) {
        h->table[hash] = htable_entry_new(key, value);
        h->size ++;
    } else {
        // Update existing key entry
        h->table[hash]->value = value;
    }
    // Resize if the table has become too full
    if (h->size >= h->table_size * htable_threshold)
        htable_resize(h, h->table_size * 2);
}

// Retrieve an element from a hashtable
void *htable_get(HTable *h, void *key) {
    htable_int64 hash = h->hash(key) % h->table_size;
    while ((h->table[hash] == HTABLE_DELETED ||
            (h->table[hash] != NULL &&
             h->table[hash]->key != key))) {
        hash = (hash + 1) % h->table_size;
    }
    if (h->table[hash] == NULL)
        return NULL;
    else
        return h->table[hash]->value;
}


// Given two set-like htables, return the set intersection
HTable *htable_intersect(HTable *h1, HTable *h2) {
    htable_int64 smallest = min(h1->table_size, h2->table_size),
                 largest = max(h1->table_size, h2->table_size);
    htable_resize(h1, largest);
    htable_resize(h2, largest);
    HTable *h = htable_new(smallest, htable_quickhash64);
    
    HTableEntry *entry;
    for (htable_int64 i = 0; i < largest; i++)
        if (((entry = h1->table[i]) != NULL &&
                entry != HTABLE_DELETED) &&
                htable_get(h2, entry->key) == entry->value)
            htable_put(h, entry->key, entry->value);

    return h;
}

// Given two set-like htables, return the set union
HTable *htable_union(HTable *h1, HTable *h2) {
    HTable *h = htable_new(h1->table_size + h2->table_size, htable_quickhash64);

    HTableEntry *entry;
    for (htable_int64 i = 0; i < h1->table_size; i++)
        if ((entry = h1->table[i])->key != NULL &&
                entry != HTABLE_DELETED)
            htable_put(h, entry->key, entry->value);
    for (htable_int64 i = 0; i < h2->table_size; i++)
        if ((entry = h2->table[i])->key != NULL &&
                entry != HTABLE_DELETED)
            htable_put(h, entry->key, entry->value);

    return h;
}


// Resize a htable to a new size
void htable_resize(HTable *h, htable_int64 size) {
    // If the given size is too small or no different to current
    if (size < h->size || size == h->table_size)
        return;

    htable_int64 old_size = h->table_size;
    HTableEntry **old_table = h->table;

    h->table = (HTableEntry **) calloc(sizeof(*h->table), size);
    h->table_size = size;
    h->size = 0;

    HTableEntry *entry;
    for (htable_int64 i = 0; i < old_size; i ++)
        if ((entry = old_table[i]) != NULL &&
                entry != HTABLE_DELETED) {
            htable_put(h, entry->key, entry->value);
            free(entry);
        }
    free(old_table);
}

// Delete an element from a htable
void htable_remove(HTable *h, void *key) {
    htable_int64 hash = h->hash(key) % h->table_size;
    while (h->table[hash] == HTABLE_DELETED ||
            (h->table[hash] != NULL &&
             h->table[hash]->key != key)) {
        hash = (hash + 1) % h->table_size;
    }
    if (h->table[hash] != NULL) {
        free(h->table[hash]);
        h->table[hash] = HTABLE_DELETED;
        h->size --;
    }
}
