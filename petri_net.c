#include "petri_net.h"

// TODO: Sort tokens (y, x, z) -> (x, y, z) and use only 'diagonal' of matrix
//       -> this is crazy good
// TODO: Expand matrix on dimension increase rather than copy
// TODO: Substitute any absolute proofs of subformulae for T


// Create a new dynamically-allocated petri net
PetriNet *petri_net_new(Formula *formula) {
    PetriNet *net = (PetriNet *) malloc(sizeof *net);
    *net = (PetriNet) {
        .symbols = formula_flatten(formula),
        .len = formula_length(formula),
        .tokens = llist_new(), //TODO: rbtree
        .places = ndarray_new(0, NULL, sizeof(bool))
    };
    return net;
}

// Free a dynamically-allocated petri net
void petri_net_free(PetriNet *net) {
    free(net);
}


// Given a petri net with tokens in n dims, extend to n+1 dims
LList *petri_net_nd_extrapolate(PetriNet *net) {
    LList *new_tokens = llist_new();
    int *old_token, *new_token, n = net->len;
    // Extrapolate old tokens 'sideways' into new dimension
    for (LListNode *tk = net->tokens->head; tk != NULL; tk = tk->next) {
        old_token = (int *) tk->element;
        for (size_t i = 0; i < n; i++) {
            new_token = (int *) calloc(sizeof(*new_token), n);
            memcpy(new_token, old_token, sizeof(int) * (n-1));
            new_token[n-1] = i;
            llist_append(new_tokens, new_token);
        }
    }
    return new_tokens;
}

// Given a petri net, create tokens for a 2D case
LList *petri_net_2d_spawn(PetriNet *net) {
    LList *new_tokens = llist_new();
    Formula *symbolA, *symbolB;
    int *new_token;
    for (size_t i = 0; i < net->len; i++) {
        symbolA = net->symbols[i];
        for (size_t j = 0; j < net->len; j++) {
            symbolB = net->symbols[j];
            if ((symbolA->type == Atom && symbolB->type == NotAtom) ||
                    (symbolA->type == NotAtom && symbolB->type == NotAtom)) {
                new_token = (int *) calloc(sizeof(*new_token), 2);
                new_token[0] = i; new_token[1] = j;
                llist_append(new_tokens, new_token);
            }
        }
    }
    return new_tokens;
}

// Perform coalescence algorithm along a single dimension axis
bool petri_net_1d_coalescence(PetriNet *net, size_t dimn, size_t *place) {
    // size_t *place is a n-large swapspace for storing place coords in
    bool fired = false, has_sibling;
    size_t *token;
    LListNode *list_node, *next_node;
    Formula *this_sym, *parent_sym, *sibling_sym;
    
    // For each token in the net
    list_node = net->tokens->head;
    while (list_node != NULL) {
        next_node = list_node->next;
        
        // Get the parent transition and this child's sibling
        memcpy(place, token, sizeof(*place) * net->places->n);
        token = (size_t *) list_node->element;
        this_sym = net->symbols[token[dimn]];
        parent_sym = this_sym->parent;
        place[dimn] = parent_sym->i;
        
        if (*(bool *) ndarray_elem(net->places, place)) {
            // Delete if the parent has been spawned already
            llist_remove_node(net->tokens, list_node);
        } else {
            // Perform the transition if the sibling exists or if performing Or
            sibling_sym = (parent_sym->left == this_sym) ? parent_sym->right : parent_sym->left;
            place[dimn] = sibling_sym->i;
            if ((has_sibling = *(bool *) ndarray_elem(net->places, place)) ||
                    parent_sym->type == Or) {
                place[dimn] = parent_sym->i;
                memcpy(token, place, sizeof(*place) * net->places->n);
                *(bool *) ndarray_elem(net->places, place) = true;
                llist_append_node(net->tokens, list_node);
                fired = true;
            }
        }

        list_node = next_node;
    }
    return fired;
}


// Perform coalescence algorithm in n dimensions
int petri_net_coalescence(PetriNet *net) {
    size_t *dims = (size_t *) malloc(sizeof(*dims)),
           *place = (size_t *) malloc(sizeof(*place));
    LListNode *tk_node;

    for (size_t n = 1; ; n++) {
        // Create new n-dim array
        ndarray_free(net->places);
        dims = (size_t *) realloc(dims, sizeof(*dims) * n);
        place = (size_t *) realloc(dims, sizeof(*place) * n);
        dims[n-1] = net->len;
        net->places = ndarray_new(n, dims, sizeof(bool));

        // Create new token queue
        while ((tk_node = net->tokens->head) != NULL) {
            free(tk_node->element);
            llist_remove_head(net->tokens);
        }

        // See petri_net_2d_deduce

        // Try to fire in all n dimensions, halt if none fired`
        bool fired = true;
        while (fired) {
            fired = false;
            for (size_t dimn = 0; dimn < n; dimn++)
                // TODO: Return [0, 2] (no fire, fire, root)
                fired |= petri_net_1d_coalescence(net, dimn, place);
        }

        // TODO: Check we halted at the root
        // * Exactly one token?
        // * Do we spawn or fire?
 
        return n;
    }
}


// Prettyprint a petri net
void petri_net_print(PetriNet *net) {
    printf("Petri Net at %p\n", net);

    for (int i = 0; i < net->places->dims[0]; i++) {
        formula_print(net->symbols[i]);
        printf(" ");
    }
    printf("\n");

    llist_print(net->tokens);
    printf("\n");

    ndarray_print(net->places);
}


//int petri_net_main(int argc, char *argv[]) {
int main(int argc, char *argv[]) {
    char **string = &argv[1];
    Formula *formula = formula_parse(string);
    PetriNet *net = petri_net_new(formula);
    int n = petri_net_coalescence(net);
    printf("%d\n", n);

    return 0;
}
