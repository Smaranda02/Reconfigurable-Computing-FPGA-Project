#include "merge_sort_naive.hpp"

// Read one element from the correct run buffer
static int run_peek(
    int run0[MAX_N], int run1[MAX_N],
    int run2[MAX_N], int run3[MAX_N],
    int pos[K], int sizes[K], int r)
{
    if (r < 0 || r >= K) return INF_VAL;
    int p = pos[r];
    int s = sizes[r];
    if (p >= s) return INF_VAL;
    switch (r) {
        case 0: return run0[p];
        case 1: return run1[p];
        case 2: return run2[p];
        case 3: return run3[p];
        default: return INF_VAL;
    }
}

// Top-level kernel
void merge_sort_naive(
    int run0[MAX_N],
    int run1[MAX_N],
    int run2[MAX_N],
    int run3[MAX_N],
    int sizes[K],
    int output[K * MAX_N],
    int *out_size)
{
    //  AXI-Lite control interface for all scalar ports 
#pragma HLS INTERFACE s_axilite port=sizes    bundle=control
#pragma HLS INTERFACE s_axilite port=out_size bundle=control
#pragma HLS INTERFACE s_axilite port=return   bundle=control

    //  AXI master (memory-mapped) for array ports 
    // Each run gets its own bundle to allow parallel reads later (optimized version)
#pragma HLS INTERFACE m_axi port=run0   offset=slave bundle=gmem0 depth=MAX_N
#pragma HLS INTERFACE m_axi port=run1   offset=slave bundle=gmem1 depth=MAX_N
#pragma HLS INTERFACE m_axi port=run2   offset=slave bundle=gmem2 depth=MAX_N
#pragma HLS INTERFACE m_axi port=run3   offset=slave bundle=gmem3 depth=MAX_N
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem4 depth=K*MAX_N

    
    // Step 1: load runs into local BRAM buffers. 
    // (no PIPELINE pragma here - naive version does this sequentially)
    
    int local0[MAX_N];
    int local1[MAX_N];
    int local2[MAX_N];
    int local3[MAX_N];

    int s0 = sizes[0];
    int s1 = sizes[1];
    int s2 = sizes[2];
    int s3 = sizes[3];

LOAD_0: for (int i = 0; i < s0; i++) local0[i] = run0[i]; 
LOAD_1: for (int i = 0; i < s1; i++) local1[i] = run1[i];
LOAD_2: for (int i = 0; i < s2; i++) local2[i] = run2[i];
LOAD_3: for (int i = 0; i < s3; i++) local3[i] = run3[i];

    
    // Step 2: build loser tree
    // loser[0]   = overall winner (index of run with current minimum)
    // loser[1..K-1] = loser at each internal node
    // wtree[K..2K-1] = leaves (run indices)
    
    int pos[K]   = {0, 0, 0, 0};   // current read position per run
    int loser[K];                   // loser tree internal nodes
    int wtree[2 * K];               // temporary winner tree for build

    // initialise leaves
BUILD_LEAVES: for (int i = 0; i < K; i++) {
        wtree[K + i] = i;
    }

    // build bottom-up (no pragma — naive sequential)
BUILD_TREE: for (int i = K - 1; i >= 1; i--) {
        int L = wtree[2 * i];
        int R = wtree[2 * i + 1];

        int vL, vR;
        // peek run L
        switch (L) {
            case 0: vL = (pos[0] < s0) ? local0[pos[0]] : INF_VAL; break;
            case 1: vL = (pos[1] < s1) ? local1[pos[1]] : INF_VAL; break;
            case 2: vL = (pos[2] < s2) ? local2[pos[2]] : INF_VAL; break;
            default: vL = (pos[3] < s3) ? local3[pos[3]] : INF_VAL; break;
        }
        // peek run R
        switch (R) {
            case 0: vR = (pos[0] < s0) ? local0[pos[0]] : INF_VAL; break;
            case 1: vR = (pos[1] < s1) ? local1[pos[1]] : INF_VAL; break;
            case 2: vR = (pos[2] < s2) ? local2[pos[2]] : INF_VAL; break;
            default: vR = (pos[3] < s3) ? local3[pos[3]] : INF_VAL; break;
        }

        if (vL <= vR) { wtree[i] = L; loser[i] = R; }
        else          { wtree[i] = R; loser[i] = L; }
    }
    loser[0] = wtree[1];   // overall winner

    
    // Step 3: merge loop — extract minimum one element at a time
    // No PIPELINE pragma — HLS will schedule this as a sequential FSM
    
    int total = s0 + s1 + s2 + s3;
    int out_idx = 0;

MERGE_LOOP: for (int step = 0; step < K * MAX_N; step++) {
        if (step >= total) break;   // done

        int w = loser[0];   // current winner = run with minimum

        // peek winner's current value
        int cur_val;
        switch (w) {
            case 0: cur_val = (pos[0] < s0) ? local0[pos[0]] : INF_VAL; break;
            case 1: cur_val = (pos[1] < s1) ? local1[pos[1]] : INF_VAL; break;
            case 2: cur_val = (pos[2] < s2) ? local2[pos[2]] : INF_VAL; break;
            default: cur_val = (pos[3] < s3) ? local3[pos[3]] : INF_VAL; break;
        }

        if (cur_val == INF_VAL) break;

        // output the minimum
        output[out_idx++] = cur_val;
        pos[w]++;   // advance that run

        // adjust loser tree from leaf w upward
        int cur  = w;
        int node = (K + w) >> 1;

ADJUST_LOOP: for (int lvl = 0; lvl < 3; lvl++) {   // log2(4) = 2 levels, +1 guard
            if (node < 1) break;

            int lval, cval;
            // value of loser at this node
            int l = loser[node];
            switch (l) {
                case 0: lval = (pos[0] < s0) ? local0[pos[0]] : INF_VAL; break;
                case 1: lval = (pos[1] < s1) ? local1[pos[1]] : INF_VAL; break;
                case 2: lval = (pos[2] < s2) ? local2[pos[2]] : INF_VAL; break;
                default: lval = (pos[3] < s3) ? local3[pos[3]] : INF_VAL; break;
            }
            // value of current contender
            switch (cur) {
                case 0: cval = (pos[0] < s0) ? local0[pos[0]] : INF_VAL; break;
                case 1: cval = (pos[1] < s1) ? local1[pos[1]] : INF_VAL; break;
                case 2: cval = (pos[2] < s2) ? local2[pos[2]] : INF_VAL; break;
                default: cval = (pos[3] < s3) ? local3[pos[3]] : INF_VAL; break;
            }

            if (cval > lval) {
                // cur loses — swap
                int tmp    = loser[node];
                loser[node] = cur;
                cur        = tmp;
            }
            node >>= 1;
        }
        loser[0] = cur;
    }

    *out_size = out_idx;
}