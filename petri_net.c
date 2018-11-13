#include "petri_net.h"

// TODO: Expand matrix on dimension increase rather than copy
// TODO: Substitute any absolute proofs of subformulae for T


size_t global_fn_dimn;
int lex_arr_cmp(RBTree *tree, RBTreeNode *left, RBTreeNode *right) {
    size_t *left_arr = (size_t *) left->value, *right_arr = (size_t *) right->value;
    for (size_t i = 0; i < global_fn_dimn; i++) {
        if (left_arr[i] < right_arr[i]) {
            return -1;
        } else if (left_arr[i] > right_arr[i]) {
            return 1;
        }
    }
    return 0;
}

void print_arr(void *arr){
    printf("[");
    for (size_t i = 0; i < global_fn_dimn; i++) {
        printf("%lu", ((size_t *) arr)[i]);
        if (i < global_fn_dimn - 1)
            printf(" ");
    }
    printf("] ");
}

int size_t_cmp_f(const void * a, const void * b) {
    size_t ca = *(size_t *) a, cb = *(size_t *) b;
    return ca > cb ? 1 : ca < cb ? -1 : 0;
}

void petri_net_token_sort(size_t *arr, size_t n) {
    qsort(arr, n, sizeof(*arr), size_t_cmp_f);
}


// Create a new dynamically-allocated petri net
PetriNet *petri_net_new(Formula *formula) {
    PetriNet *net = (PetriNet *) malloc(sizeof *net);
    *net = (PetriNet) {
        .symbols = formula_flatten(formula),
        .len = formula_length(formula),
        .tokens = rbtree_new(lex_arr_cmp),
        .places = ndarray_new(0, NULL, sizeof(bool))
    };
    return net;
}

// Free a dynamically-allocated petri net
void petri_net_free(PetriNet *net) {
    free(net->symbols);
    rbtree_free(net->tokens, rbtree_node_ptr_dealloc_cb);
    if (net->places != NULL) ndarray_free(net->places);
    free(net);
}


// Given a petri net with tokens in n dims, extend to n+1 dims
RBTree *petri_net_nd_extrapolate(PetriNet *net) {
    RBTree *new_tokens = rbtree_new(lex_arr_cmp);
    size_t *old_tk, *new_tk, n = net->places->n;
    // Extrapolate old tokens 'sideways' into new dimension
    RBTreeIter *iter = rbtree_iter_new();
    for (old_tk = (size_t *) rbtree_iter_first(iter, net->tokens); old_tk; old_tk = (size_t *) rbtree_iter_next(iter)) {
        for (size_t i = 0; i < net->len; i++) {
            new_tk = (size_t *) calloc(sizeof(*new_tk), n);
            memcpy(new_tk, old_tk, sizeof(*new_tk) * (n-1));
            new_tk[n-1] = i;
            petri_net_token_sort(new_tk, n);
            if (rbtree_find(new_tokens, new_tk)) {
                free(new_tk);
            } else {
                rbtree_insert(new_tokens, new_tk);
            }
        }
    }
    rbtree_iter_free(iter);
    return new_tokens;
}

// Given a petri net, create tokens for a 2D case
RBTree *petri_net_2d_spawn(PetriNet *net) {
    RBTree *new_tokens = rbtree_new(lex_arr_cmp);
    Formula *symbolA, *symbolB;
    size_t *new_tk;
    for (size_t i = 0; i < net->len; i++) {
        symbolA = net->symbols[i];
        for (size_t j = i; j < net->len; j++) {
            symbolB = net->symbols[j];
            if (((symbolA->type == Atom && symbolB->type == NotAtom) ||
                        (symbolA->type == NotAtom && symbolB->type == Atom)) &&
                    (symbolA->symbol == symbolB->symbol)) {
                new_tk = (size_t *) calloc(sizeof(*new_tk), 2);
                new_tk[0] = i; new_tk[1] = j;
                petri_net_token_sort(new_tk, 2);
                if (rbtree_find(new_tokens, new_tk)) {
                    free(new_tk);
                } else {
                    rbtree_insert(new_tokens, new_tk);
                }
            }
        }
    }
    return new_tokens;
}

// Perform coalescence algorithm along a single dimension axis
bool petri_net_1d_coalescence(PetriNet *net, size_t dimn, size_t *place) {
    // size_t *place is a n-large swapspace for storing place coords in
    bool fired = false;
    size_t *token, n = net->places->n;
    Formula *this_sym, *parent_sym, *sibling_sym;

    // For each token in the net
    RBTreeIter *iter = rbtree_iter_new();
    for (token = (size_t *) rbtree_iter_first(iter, net->tokens); token; token = (size_t *) rbtree_iter_next(iter)) {
        this_sym = net->symbols[token[dimn]];

        if (this_sym->parent == NULL) {
            // Skip if we're at a partial root
            continue;

        } else {
            // Get the parent transition
            parent_sym = this_sym->parent;
            memcpy(place, token, sizeof(*place) * n);
            place[dimn] = parent_sym->i;
            petri_net_token_sort(place, n);

            if (*(bool *) ndarray_elem(net->places, place)) {
                // Delete if the parent has been spawned already
                // TODO: Has to be checked in all dimensions?
                Formula *this_sym_check, *parent_sym_check;
                bool check = true;
                for (size_t dimn_check = 0; dimn_check < n; dimn_check++) {
                    this_sym_check = net->symbols[token[dimn_check]];
                    parent_sym_check = this_sym_check->parent;
                    check &= (parent_sym_check != NULL);
                    if (check) {
                        memcpy(place, token, sizeof(*place) * n);
                        place[dimn_check] = parent_sym_check->i;
                        petri_net_token_sort(place, n);
                        check &= *(bool *) ndarray_elem(net->places, place);
                    }
                }
                if (check)
                    // FIXME: This should really be added to a list and removed later
                    rbtree_remove(net->tokens, token);

            } else {
                // Remove node from tree but maintain token
                sibling_sym = (parent_sym->left == this_sym) ? parent_sym->right : parent_sym->left;
                memcpy(place, token, sizeof(*place) * n);
                place[dimn] = sibling_sym->i;
                petri_net_token_sort(place, n);

                // Perform the transition if the sibling exists or if performing Or
                if ((*(bool *) ndarray_elem(net->places, place)) ||
                        parent_sym->type == Or) {
                    // FIXME: Again, store and remove later
                    rbtree_remove_with_cb(net->tokens, token, rbtree_node_dealloc_cb);
                    token[dimn] = parent_sym->i;
                    petri_net_token_sort(token, n);
                    *(bool *) ndarray_elem(net->places, token) = true;
                    rbtree_insert(net->tokens, token);
                    fired = true;
                }
            }
        }
    }
    rbtree_iter_free(iter);

    return fired;
}


// Perform coalescence algorithm in n dimensions
size_t petri_net_coalescence(PetriNet *net) {
    size_t *dims = (size_t *) malloc(sizeof(*dims)),
           *place = (size_t *) malloc(sizeof(*place)),
           *root = (size_t *) malloc(sizeof(*root));
    RBTree *new_tokens;
    dims[0] = net->len;
    Formula *maybe_root = net->symbols[0];
    while (maybe_root->parent != NULL)
        maybe_root = maybe_root->parent;
    root[0] = maybe_root->i;

    for (size_t i = 0; i < net->len; i++) {
        //printf("[%lu] = ", i); formula_print(net->symbols[i]); printf("\n");
    }

    for (size_t n = 2; n < net->len; n++) {
        global_fn_dimn = n;

        // Create new n-dim array
        ndarray_free(net->places);
        dims = (size_t *) realloc(dims, sizeof(*dims) * n);
        place = (size_t *) realloc(place, sizeof(*place) * n);
        root = (size_t *) realloc(root, sizeof(*root) * n);
        dims[n-1] = net->len;
        root[n-1] = root[n-2];
        net->places = ndarray_new(n, dims, sizeof(bool));
        
        // Create new token tree
        if (n == 2) {
            new_tokens = petri_net_2d_spawn(net);
        } else {
            new_tokens = petri_net_nd_extrapolate(net);
        }
        rbtree_free(net->tokens, rbtree_node_ptr_dealloc_cb);
        net->tokens = new_tokens;

        // Populate grid
        RBTreeIter *iter = rbtree_iter_new();
        for (size_t *token = (size_t *) rbtree_iter_first(iter, net->tokens); token; token = (size_t *) rbtree_iter_next(iter)) {
            *(bool *) ndarray_elem(net->places, token) = true;
        }
        rbtree_iter_free(iter);

        // Try to fire in all n dimensions, halt if none fired
        bool fired = true;
        while (fired) {
            fired = false;
            for (size_t dimn = 0; dimn < n; dimn++) {
                fired |= petri_net_1d_coalescence(net, dimn, place);
            }
            if (rbtree_find(net->tokens, root)) {
                free(dims); free(place); free(root);
                return n;
            }
        }
    }
    
    free(dims); free(place); free(root);
    return -1;
}


// Prettyprint a petri net
void petri_net_print(PetriNet *net) {
    printf("~~ Petri Net at %p\n", net);

    for (int i = 0; i < net->places->dims[0]; i++) {
        formula_print(net->symbols[i]);
        printf(" ");
    }
    printf("\n");

    rbtree_print(net->tokens, print_arr);
    printf("\n");

    ndarray_print(net->places);
    printf("\n~~ End of Petri Net\n");
}


//int petri_net_main(int argc, char *argv[]) {
int main(int argc, char *argv[]) {
    char **string = &argv[1];
    Formula *formula = formula_parse(string);
    PetriNet *net = petri_net_new(formula);
    int n = petri_net_coalescence(net);
    printf("%d\n", n);
    
    petri_net_free(net);
    formula_free(formula);
    return 0;
}
