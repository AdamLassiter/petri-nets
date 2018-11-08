#include "petri_net.h"

// TODO: Sort tokens (y, x, z) -> (x, y, z) and use only 'diagonal' of matrix
// TODO: Expand matrix on dimension increase rather than copy


// Create a new dynamically-allocated petri net
PetriNet *petri_net_new(Formula *formula) {
    PetriNet *net = (PetriNet *) malloc(sizeof *net);
    *net = (PetriNet) {
        .symbols = formula_flatten(formula),
        .len = formula_length(formula),
        .tokens = llist_new(),
        .places = ndarray_new(0, NULL, sizeof(bool))
    };
    return net;
}

// Free a dynamically-allocated petri net
void petri_net_free(PetriNet *net) {
    free(net);
}


// Given a petri net with tokens in (n-1) dims, extend to n dims
void petri_net_extend_tokens(PetriNet *net, size_t n) {
    LList *new_tokens = llist_new();
    int *old_token, *new_token;
    // Extrapolate old tokens 'sideways' into new dimension
    for (LListNode *tk = net->tokens->head; tk != NULL; tk = tk->next) {
        old_token = (int *) tk->element;
        for (size_t i = 0; i < net->len; i++) {
            new_token = (int *) calloc(sizeof(*new_token), n);
            memcpy(new_token, old_token, sizeof(int) * (n-1));
            //new_token[n-1] = net->places[];
            llist_append(new_tokens, new_token);
        }
    }
}
/* nD Extrapolation
 * [x1 .. xn-1] => [x1 .. xn-1 y] forall y
 *
 * 2D Re-Deduction
 * [.. a ..] => [.. a .. ~a]
 * */


// Perform coalescence algorithm along a single dimension axis
bool petri_net_1d_coalescence(PetriNet *net, size_t dimn, size_t *place) {
    // size_t *place is a n-large swapspace for storing place coords in
    bool fired = false;
    size_t *token;
    LListNode *list_node, *next_node, *sibling_node;
    Formula *dim_sym, *next_sym, *sibling_sym;
    
    // For each token in the net
    list_node = net->tokens->head;
    while (list_node != NULL) {
        next_node = list_node->next;
        
        // Get the parent transition and this child's sibling
        // TODO: Delete if the parent exists
        token = (size_t *) list_node->element;
        dim_sym = net->symbols[token[dimn]];
        next_sym = dim_sym->parent;
        sibling_sym = (next_sym->left == dim_sym) ? next_sym->right : next_sym->left;
        
        // Perform the transition on both children if the sibling exists
        // Perform the transition on just this child if performing Or
        memcpy(place, token, sizeof(*place) * net->places->n);
        place[dimn] = sibling_sym->i;
        if ((sibling_node = (LListNode *) ndarray_elem(net->places, place)) != NULL ||
                next_sym->type == Or) {
            if (sibling_node != NULL) {
                free(sibling_node->element);
                llist_remove_node(net->tokens, sibling_node);
            }
            place[dimn] = next_sym->i;
            memcpy(token, place, sizeof(*place) * net->places->n);
            *(LListNode **) ndarray_elem(net->places, place) = next_node;
            llist_append_node(net->tokens, list_node);
            fired = true;
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
        net->places = ndarray_new(n, dims, sizeof(LListNode *));

        // Create new token queue
        while ((tk_node = net->tokens->head) != NULL) {
            free(tk_node->element);
            llist_remove_head(net->tokens);
        }

        // TODO: Prove T :: |- a, ¬a
        // See petri_net_extend_tokens

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
