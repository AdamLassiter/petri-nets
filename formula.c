#include "formula.h"


// Create and initialise a dynamically-allocated formula tree from a string
Formula *formula_parse(char **string) {
    Formula *formula = (Formula *) malloc(sizeof *formula);
    *formula = (Formula) {
        .parent = NULL,
        .left = NULL,
        .right = NULL
    };
    switch (*string[0]) {
        case '(':
            *string += sizeof(char);
            formula->left = formula_parse(string);
            formula->left->parent = formula;
            switch(*string[0]) {
                case Or:
                case And:
                    formula->symbol = *string[0];
                    formula->type = (Grammar) *string[0];
                break;
                default:
                    fprintf(stderr,
                            "Error parsing string: unexpected %c, expected %c,%c\n",
                            *string[0], Or, And);
                    exit(1);
                break;
            }
            *string += sizeof(char);
            formula->right = formula_parse(string);
            formula->right->parent = formula;
            if (*string[0] != ')') { 
                    fprintf(stderr,
                            "Error parsing string: unexpected %c, expected )\n",
                            *string[0]);
                    exit(1);
            }
        break;
        case Or:
        case And:
            fprintf(stderr,
                    "Error parsing string: unexpected %c, expected %c,%c,a,%ca\n",
                    *string[0], Top, Bottom, NotAtom);
            exit(1);
        case Top:
        case Bottom:
            formula->symbol = *string[0];
            formula->type = (Grammar) *string[0];
        break;
        case NotAtom:
            *string += sizeof(char);
            switch (*string[0]) {
                case '(':
                case ')':
                case Or:
                case And:
                case Top:
                case Bottom:
                case NotAtom:
                    fprintf(stderr,
                            "Error parsing string: unexpected %c, expected a\n",
                            *string[0]);
                    exit(1);
                break;
                default:
                    formula->symbol = *string[0];
                    formula->type = NotAtom;
                break;
            }
        break;
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
    if (formula->left != NULL) {
        formula_free(formula->left);
    }
    if (formula->right != NULL) {
        formula_free(formula->right);
    }
    free(formula);
}


// Get the length of a formula
size_t formula_length(Formula *formula) {
    if (formula->type == Atom || formula->type == NotAtom) {
        return 1;
    } else {
        return 1 + formula_length(formula->left) + formula_length(formula->right);
    }
}

// Performs a recursive walk on the tree with no initals nor conversion of llist
int formula_symbol_walk(Formula *formula, LList *list, size_t i) {
    if (formula->type == Or || formula->type == And) {
        i = formula_symbol_walk(formula->left, list, i);
        formula->i = i;
        llist_append(list, formula);
        return formula_symbol_walk(formula->right, list, i+1);
    } else {
        formula->i = i;
        llist_append(list, formula);
        return i+1;
    }
}

// Given a formula tree, return a left->right walk of nodes
Formula **formula_flatten(Formula *formula) {
    LList *list = llist_new();
    Formula **array = (Formula **) calloc(sizeof(*array), formula_symbol_walk(formula, list, 0));

    LListNode *node = list->head;
    for (int i = 0; i < list->len; i++) {
        array[i] = (Formula *) node->element;
        node = node->next;
    }

    llist_free(list);
    return array;
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
        char **string = &argv[1];
        Formula *formula = formula_parse(string);
        formula_print(formula);
    } else {
        fprintf(stderr, "Error running formula: incorrect number of args\n");
    }
    return 0;
}
