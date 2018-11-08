#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef RB_ITER_MAX_HEIGHT
#define RB_ITER_MAX_HEIGHT 64 // Tallest allowable tree to iterate
#endif

struct RBTree;
struct RBTreeNode;

typedef int (*rbtree_node_cmp_f) (struct RBTree *, struct RBTreeNode *, struct RBTreeNode *);
typedef void (*rbtree_node_f) (struct RBTree *tree, struct RBTreeNode *node);

typedef struct RBTreeNode {
    bool red;
    void *value;
    struct RBTreeNode *link[2];
} RBTreeNode;

typedef struct RBTree {
    RBTreeNode *root;
    rbtree_node_cmp_f cmp;
    size_t size;
} RBTree;

struct rbtree_iter {
    RBTree *tree;
    RBTreeNode *node;
    RBTreeNode *path[RB_ITER_MAX_HEIGHT];
    size_t top;
};


RBTree *rbtree_new(rbtree_node_cmp_f);
void rbtree_free(RBTree *, rbtree_node_f);

size_t rbtree_size(RBTree *);
void *rbtree_find(RBTree *, void *);

bool rbtree_insert(RBTree *, void *);
bool rbtree_remove_with_cb(RBTree *, void *, rbtree_node_f);
bool rbtree_remove(RBTree *, void *);
