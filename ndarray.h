#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct NdArray {
    size_t n, *dims, size;
    unsigned char *array;
} NdArray;


NdArray *ndarray_new(size_t, size_t [], size_t);
void ndarray_free(NdArray *);

void *ndarray_elem(NdArray *, size_t []);

void ndarray_print(NdArray *);
