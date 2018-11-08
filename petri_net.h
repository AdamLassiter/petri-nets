#pragma once

#include "formula.h"
#include "ndarray.h"
#include "llist.h"

#include <stdlib.h>


typedef struct PetriNet {
    Formula **symbols;
    size_t len;
    LList *tokens;
    NdArray *places;
} PetriNet;


PetriNet *petri_net_new(Formula *);
void petri_net_free(PetriNet *);

int petri_net_coalescence(PetriNet *);

void petri_net_print(PetriNet *);
