#pragma once
#include <ap_int.h>

// Fixed parameters for naive FPGA implementation
#define K         4       // number of sorted runs
#define MAX_N     4096    // max elements per run (total = K * MAX_N = 16384)
#define INF_VAL   0x7FFFFFFF

void merge_sort_naive(
    int run0[MAX_N],
    int run1[MAX_N],
    int run2[MAX_N],
    int run3[MAX_N],
    int sizes[K],       // actual size of each run
    int output[K * MAX_N],
    int *out_size
);