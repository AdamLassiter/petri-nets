#include "formula.h"


// Create and initialise a dynamically-allocated formula tree from a string
Formula *formula_parse(char **string) {
    Formula *formula = (Formula *) malloc(sizeof *formula);
    char *op, *left_bt, *right_bt, op_bt;

    *formula = (Formula) {
        .parent = NULL,
        .left = NULL,
        .right = NULL
    };

    switch (*string[0]) {
        // Binary Operators
        case '(':
            left_bt = *string;
            right_bt = *string;

            *string += sizeof(char);
            formula->left = formula_parse(string);
            formula->left->parent = formula;

            op = *string;

            *string += sizeof(char);
            formula->right = formula_parse(string);
            formula->right->parent = formula;

            switch (*op) {
                // Binary And/Or
                case Or:
                case And:
                    formula->symbol = *op;
                    formula->type = (Grammar) *op;
                break;
                
                // Implies / Implied by (¬a v b) / (a v ¬b)
                case Implies:
                case Implied:
                    formula->symbol = Or;
                    formula->type = Or;
                    formula_negate(*op == Implies? formula->left : formula->right);
                break;
                
                // Equivalence by (a -> b) ^ (a <- b)
                case NotEquivalent:
                case Equivalent:
                    op_bt = *op;
                    // Top-level construction
                    formula->symbol = And;
                    formula->type = And;
                    // Left ->
                    *op = Implies;
                    formula_free(formula->left);
                    formula->left = formula_parse(&left_bt);
                    formula->left->parent = formula;
                    // Right <-
                    *op = Implied;
                    formula_free(formula->right);
                    formula->right = formula_parse(&right_bt);
                    formula->right->parent = formula;
                    // Fix what was mangled
                    *op = op_bt;
                    if (*op == NotEquivalent) formula_negate(formula);
                break;

                default:
                    fprintf(stderr,
                            "Error parsing string: unexpected %c, expected %c,%c\n",
                            *string[0], Or, And);
                    exit(1);
                break;
            }
            if (*string[0] != ')') {
                fprintf(stderr,
                        "Error parsing string: unexpected %c, expected )\n",
                        *string[0]);
                exit(1);
            }
        break;
        
        // Unary Not
        case NotAtom:
            *string += sizeof(char);
            if (*string[0] == '(') {
                free(formula);
                formula = formula_parse(string);
                formula_negate(formula);
                *string -= sizeof(char);
            } else {
                formula->symbol = *string[0];
                formula->type = NotAtom;
            }
        break;
        
        // Out-of-place Binary Operator
        case Or:
        case And:
        case Implies:
        case Implied:
        case Equivalent:
            fprintf(stderr,
                    "Error parsing string: unexpected %c, expected %c,%c,a,%ca\n",
                    *string[0], Top, Bottom, NotAtom);
            exit(1);
        break;
        
        // Top/Bottom
        case Top:
        case Bottom:
            formula->symbol = *string[0];
            formula->type = (Grammar) *string[0];
        break;
        
        // Atom
        default:
            formula->symbol = *string[0];
            formula->type = Atom;
        break;
    }
    *string += sizeof(char);
    return formula;
}

// Free a dynamically-allocated formula
void formula_free(Formula *formula) {
    if (formula != NULL) {
        formula_free(formula->left);
        formula_free(formula->right);
        free(formula);
    }
}


// Reindex a formula tree
size_t formula_index(Formula *formula, size_t i) {
    if (formula->type == Or || formula->type == And) {
        i = formula_index(formula->left, i);
        formula->i = i;
        return formula_index(formula->right, ++i);
    } else {
        formula->i = i;
        return ++i;
    }
}

// Get the length of a formula
size_t formula_length(Formula *formula) {
    return formula_index(formula, 0); 
}

// Performs a recursive walk on the tree with no initals nor conversion of llist
void formula_symbol_walk(Formula *formula, LList *list) {
    if (formula->type == Or || formula->type == And) {
        formula_symbol_walk(formula->left, list);
        llist_append(list, formula);
        formula_symbol_walk(formula->right, list);
    } else {
        llist_append(list, formula);
    }
}

// Given a formula tree, return a left->right walk of nodes
Formula **formula_flatten(Formula *formula) {
    LList *list = llist_new();
    formula_symbol_walk(formula, list);
    Formula **array = (Formula **) calloc(sizeof(*array), formula_length(formula));

    LListNode *node = list->head;
    for (int i = 0; i < list->len; i++) {
        array[i] = (Formula *) node->value;
        node = node->next;
    }

    llist_free(list);
    return array;
}


// Given a formula, produce the logical inverse
void formula_negate(Formula *formula) {
    switch(formula->type) {
        case And:
            formula->type = Or;
            formula->symbol = Or;
            formula_negate(formula->left);
            formula_negate(formula->right);
        break;
        case Or:
            formula->type = And;
            formula->symbol = And;
            formula_negate(formula->left);
            formula_negate(formula->right);
        break;
        case Atom:
            formula->type = NotAtom;
        break;
        case NotAtom:
            formula->type = Atom;
        break;
        case Top:
            formula->type = Bottom;
            formula->symbol = Bottom;
        break;
        case Bottom:
            formula->type = Top;
            formula->symbol = Top;
        break;
    }
}


// Prettyprint a formula
void formula_print(Formula *formula) {
    switch (formula->type) {
        case NotAtom:
            printf("~"); // Allow fallthrough
        case Top:
        case Bottom:
        case Atom:
            printf("%c", formula->symbol);
        break;
        case And:
        case Or:
           printf("(");
           formula_print(formula->left);
           printf("%c", formula->symbol);
           formula_print(formula->right);
           printf(")");
        break;
        default:
            fprintf(stderr, "Error traversing formula: malformed formula\n");
            exit(1);
        break;
    }
}


// Test for formula class, take a string input, convert it to a formula, print it
int formula_main(int argc, char *argv[]) {
//int main(int argc, char *argv[]) {
    if (argc == 2) {
        char *string = argv[1];
        Formula *formula = formula_parse(&string);
        formula_print(formula);
    } else {
        fprintf(stderr, "Error running formula: incorrect number of args\n");
    }
    return 0;
}
