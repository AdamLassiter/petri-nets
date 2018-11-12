#pragma once

#include "llist.h"
#include "rbtree.h"
#include "ndarray.h"
#include "formula.h"

#include <stdlib.h>


typedef struct PetriNet {
    Formula **symbols;
    size_t len;
    RBTree *tokens;
    NdArray *places;
} PetriNet;


PetriNet *petri_net_new(Formula *);
void petri_net_free(PetriNet *);

size_t petri_net_coalescence(PetriNet *);

void petri_net_print(PetriNet *);
