#pragma once

#include "petri_net.h"
#include "formula.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct SequentProof {
    LList *branches;
    LList *sequents;
} SequentProof;


SequentProof *sequent_proof_new();
void sequent_proof_free(SequentProof *);

SequentProof *sequent_backtrack(PetriNet *, size_t *);
PetriNet *sequent_coalescence(Formula *, bool, bool);

void sequent_proof_latex(SequentProof *);
void sequent_proof_print(SequentProof *);
