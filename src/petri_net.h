#pragma once

#include "llist.h"
#include "rbtree.h"
#include "ndarray.h"
#include "formula.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


typedef struct PetriNet {
    Formula **symbols;
    size_t len;
    RBTree *tokens;
    NdArray *places;
} PetriNet;


PetriNet *petri_net_new(Formula *);
void petri_net_free(PetriNet *);

int petri_net_coalescence(Formula *, bool);

void petri_net_print(PetriNet *);
