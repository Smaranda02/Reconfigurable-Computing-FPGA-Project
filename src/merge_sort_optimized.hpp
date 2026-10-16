#pragma once
#include <ap_int.h>

#define K         4
#define MAX_N     4096
#define LOG_K     2        // log2(4) = 2 tree levels
#define INF_VAL   0x7FFFFFFF

void merge_sort_optimized(
    int run0[MAX_N],
    int run1[MAX_N],
    int run2[MAX_N],
    int run3[MAX_N],
    int sizes[K],
    int output[K * MAX_N],
    int *out_size
);