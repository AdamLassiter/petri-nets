#include "ndarray.h"


// Create a new dynamically-allocated n-dim array
NdArray *ndarray_new(size_t n, size_t dims[], size_t s) {
    size_t size = 1;
    for (size_t i = 0; i < n; i++) {
        size *= dims[i];
    }
    size_t d_size = sizeof(*dims) * n;

    NdArray *array = (NdArray *) malloc(sizeof(*array));
    *array = (NdArray) {
        .array = (unsigned char *) calloc(size, s),
        .n = n,
        .dims = (size_t *) malloc(d_size),
        .size = s
    };
    memcpy(array->dims, dims, d_size);

    return array;
}

// Free a dynamically-allocated array
void ndarray_free(NdArray *array){
    free(array->dims);
    free(array->array);
    free(array);
}


// Return a pointer into the array at a given index
void *ndarray_elem(NdArray *array, size_t index[]) {
    if (array->n == 0) {
        return array->array;
    }

    int scalar_index = index[0];
    for (size_t n = 1; n < array->n; n++) {
        scalar_index = scalar_index * array->dims[n] + index[n];
    }

    return array->array + scalar_index * array->size;
}


// Prettyprint in n dimensions... oh boy
void ndarray_print(NdArray *array) {
    // dude, pls
    printf("N-dimensional array...");
}


// Test for nd-array class, fill an array as sums of indices, print the diagonal 
int ndarray_main(int argc, char *argv[]) {
    size_t n = 4;
    size_t d[4] = {3, 3, 3, 3};
    NdArray *array = ndarray_new(n, d, sizeof(int));
    int *elem;
    size_t index[4];
    for (index[0] = 0; index[0] < 3; index[0]++)
        for (index[1] = 0; index[1] < 3; index[1]++)
            for (index[2] = 0; index[2] < 3; index[2]++)
                for (index[3] = 0; index[3] < 3; index[3]++) {
                    elem = (int *) ndarray_elem(array, index);
                    *elem = index[0] + index[1] + index[2] + index[3];
                }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            index[j] = i;
        printf("%d ", *(int *) ndarray_elem(array, index));
    }
    printf("\n");
    return 0;
}
