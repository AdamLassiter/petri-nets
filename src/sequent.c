#include "sequent.h"


// Create a new dynamically-allocated sequent proof
SequentProof *sequent_proof_new() {
    SequentProof *proof = (SequentProof *) malloc(sizeof *proof);
    *proof = (SequentProof) {
        .branches = llist_new(),
        .sequents = llist_new()
    };
    return proof;
}

// Free a dynamically-allocated sequent proof
void sequent_proof_free(SequentProof *proof) {
    for (LListNode *n = proof->sequents->head; n != NULL; n = n->next)
        sequent_proof_free((SequentProof *) n->value);
    free(proof->branches);
    free(proof->sequents);
    free(proof);
}


// Perform coalescence algorithm in all dimensions until halt or out-of-memory error
PetriNet *sequent_coalescence(Formula *f, bool top_opt, bool partial_print) {
    int n;
    
    for (n = 2; n <= formula_n_free_names(f) + 1; n++) {
        if (partial_print) {formula_print(f); printf("\n");}

        // Fire an n-dimensional net exhaustively
        PetriNet *net = petri_net_exhaustive_fire(f, n);
        Formula *f_next = petri_net_substitute_top(net, f);

        // Check if we have reached the root
        if (f_next->type == Top) {
            formula_free(f_next);
            return net;
        }
        
        // Optimise by substituting proofs of subformulae for T
        if (top_opt) {
            formula_free(f);
            f = f_next;
        } else {
            formula_free(f_next);
        }
        petri_net_free(net);
    }

    return NULL;
}

// Given a proven petri net, backtrack to build a sequent proof
SequentProof *sequent_backtrack(PetriNet *net, size_t *place) {
    bool backtracked = false;
    size_t n = net->places->n;
    size_t *left_tk = (size_t *) calloc(sizeof *left_tk, n);
    size_t *right_tk = (size_t *) calloc(sizeof *right_tk, n);

    // Create a new sequent proof here
    SequentProof *proof = sequent_proof_new();

    for (size_t dim = 0; dim < n; dim++) {

        Formula *f = net->symbols[place[dim]];
        Grammar type = f->type;

        // Regardless of how we got here, this is proven
        llist_append(proof->sequents, f);
        
        // Check if one or both paths need to be checked
        size_t arity = type == And ? 2 : type == Or ? 1 : 0;
        bool left_child = false,
             right_child = false;
        
        // Where they exist, get both children
        if (arity > 0) { 
            // Left
            memcpy(left_tk, place, sizeof(*place) * n);
            left_tk[dim] = f->left->i;
            petri_net_token_sort(left_tk, n);
            left_child = ndarray_get(net->places, left_tk);
            // Right
            memcpy(right_tk, place, sizeof(*place) * n);
            right_tk[dim] = f->right->i;
            petri_net_token_sort(left_tk, n);
            right_child = ndarray_get(net->places, right_tk);
        }

        // Append children as tree branches
        size_t n_children = left_child && right_child ? 2 : left_child || right_child ? 1 : 0;
        // Don't follow in multiple dimensions
        if (n_children >= arity && arity > 0 && !backtracked) {
            backtracked = true;
            switch (arity) {
                case 2:
                    llist_append(proof->branches, sequent_backtrack(net, left_tk));
                    llist_append(proof->branches, sequent_backtrack(net, right_tk));
                break;
                case 1:
                    llist_append(proof->branches, sequent_backtrack(net, left_child ? left_tk : right_tk));
                break;
            }
        }
    }
    free(left_tk);
    free(right_tk);
    return proof;
}


static void sequent_print(LList *sequents) {
    for (LListNode *n = sequents->head; n != NULL; n = n->next) {
        Formula *f = (Formula *) n->value;
        formula_latex(f);
        if (n->next != NULL) printf(", ");
    }
}

void sequent_proof_print(SequentProof *proof) {
    // Count number of children to deduce inference type
    size_t n_children = 0;

    // Print leaves above
    for (LListNode *n = proof->branches->head; n != NULL; n = n->next) {
        SequentProof *q = (SequentProof *) n->value;
        sequent_proof_print(q);
        n_children ++;
    }

    // Print appropriate inference type
    switch (n_children) {
        case 2:
            printf("\\BinaryInfC{$ \\vdash ");
            break;
        case 1:
            printf("\\UnaryInfC{$ \\vdash ");
        break;
        case 0:
            printf("\\AxiomC{$ \\vdash ");
        break;
    }
    sequent_print(proof->sequents);
    printf("$}\n");
}

void sequent_proof_latex(SequentProof *proof) {
    printf("Proof of $ ");
    formula_latex(proof->sequents->head->value);
    printf("$\n\\par\n");

    printf("\\begin{prooftree}\n");
    sequent_proof_print(proof);
    printf("\\end{prooftree}\n\\par\n");
}


int main(int argc, char *argv[]) {
    bool top_optimise = false,
         print_sequent = false;
    int c;
    while ((c = getopt(argc, argv, "st")) != -1)
        switch ((char) c) {
            case 's':
                print_sequent = true;
                break;
            case 't':
                top_optimise = true;
                break;
            case '?':
                if (isprint(optopt))
                  fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                  fprintf(stderr,
                           "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort();
        }
    
    char *string = argv[optind];
    Formula *formula = formula_parse(&string);
    
    /* 3, 2, 1, Go... */
    clock_t start = clock();
    PetriNet *net = sequent_coalescence(formula, top_optimise, !print_sequent);
    clock_t diff = clock() - start;
    /* Finish */

    size_t n = net->places->n;
    
    if (print_sequent) {
        printf("\\documentclass{standalone}\n\\usepackage{bpextra}\n");
        printf("\\begin{document}\n\\begin{minipage}{1\\linewidth} %%%% CHANGEME!\n");
    }

    // Print a latex-suitable sequent proof
    if (print_sequent && n > 0) {
        // Generate a token for the root node
        size_t *root = (size_t *) calloc(sizeof *root, n);
        for (size_t i = 0; i < n; i++) {
            root[i] = formula->i;
        }
        
        // Print the proof of the formula root
        SequentProof *p = sequent_backtrack(net, root);
        sequent_proof_latex(p);

        free(root);
    }

    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf(n > 0 ? "Solution in %lu dimensions.\n" : "No solution found (up to %lu dimensions).\n", n);
    printf("Time taken: %d seconds %d milliseconds\n", msec / 1000, msec % 1000);

    if (print_sequent) printf("\\end{minipage}\n\\end{document}\n");
    
    formula_free(formula);
    return 0;
}
