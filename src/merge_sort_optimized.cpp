#include "merge_sort_optimized.hpp"

void merge_sort_optimized(
    int run0[MAX_N],
    int run1[MAX_N],
    int run2[MAX_N],
    int run3[MAX_N],
    int sizes[K],
    int output[K * MAX_N],
    int *out_size)
{
    // Interfaces: identical to naive version for fair comparison 
#pragma HLS INTERFACE s_axilite port=sizes    bundle=control
#pragma HLS INTERFACE s_axilite port=out_size bundle=control
#pragma HLS INTERFACE s_axilite port=return   bundle=control
#pragma HLS INTERFACE m_axi port=run0   offset=slave bundle=gmem0 depth=MAX_N
#pragma HLS INTERFACE m_axi port=run1   offset=slave bundle=gmem1 depth=MAX_N
#pragma HLS INTERFACE m_axi port=run2   offset=slave bundle=gmem2 depth=MAX_N
#pragma HLS INTERFACE m_axi port=run3   offset=slave bundle=gmem3 depth=MAX_N
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem4 depth=K*MAX_N

    int local0[MAX_N];
    int local1[MAX_N];
    int local2[MAX_N];
    int local3[MAX_N];
#pragma HLS ARRAY_PARTITION variable=local0 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=local1 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=local2 cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=local3 cyclic factor=2 dim=1

    // loser[] and pos[] are tiny (4 ints each) - HLS maps them to
    // registers automatically. Explicitly partition for parallel access.
    int loser[K];
    int pos[K];
#pragma HLS ARRAY_PARTITION variable=loser complete dim=1
#pragma HLS ARRAY_PARTITION variable=pos   complete dim=1

    int s0 = sizes[0];
    int s1 = sizes[1];
    int s2 = sizes[2];
    int s3 = sizes[3];

    // Init pos to zero
    for (int i = 0; i < K; i++) {
#pragma HLS UNROLL
        pos[i] = 0;
    }

   
LOAD_0: for (int i = 0; i < MAX_N; i++) {
#pragma HLS PIPELINE II=1
        local0[i] = (i < s0) ? run0[i] : INF_VAL;
    }
LOAD_1: for (int i = 0; i < MAX_N; i++) {
#pragma HLS PIPELINE II=1
        local1[i] = (i < s1) ? run1[i] : INF_VAL;
    }
LOAD_2: for (int i = 0; i < MAX_N; i++) {
#pragma HLS PIPELINE II=1
        local2[i] = (i < s2) ? run2[i] : INF_VAL;
    }
LOAD_3: for (int i = 0; i < MAX_N; i++) {
#pragma HLS PIPELINE II=1
        local3[i] = (i < s3) ? run3[i] : INF_VAL;
    }
   
    int wtree[2 * K];
#pragma HLS ARRAY_PARTITION variable=wtree complete dim=1

BUILD_LEAVES: for (int i = 0; i < K; i++) {
#pragma HLS UNROLL
        wtree[K + i] = i;
    }

BUILD_TREE: for (int i = K - 1; i >= 1; i--) {
        int L = wtree[2 * i];
        int R = wtree[2 * i + 1];
        int vL, vR;
        switch (L) {
            case 0: vL = local0[0]; break;
            case 1: vL = local1[0]; break;
            case 2: vL = local2[0]; break;
            default: vL = local3[0]; break;
        }
        switch (R) {
            case 0: vR = local0[0]; break;
            case 1: vR = local1[0]; break;
            case 2: vR = local2[0]; break;
            default: vR = local3[0]; break;
        }
        if (vL <= vR) { wtree[i] = L; loser[i] = R; }
        else          { wtree[i] = R; loser[i] = L; }
    }
    loser[0] = wtree[1];

    int total = s0 + s1 + s2 + s3;

   
MERGE_LOOP: for (int step = 0; step < K * MAX_N; step++) {
#pragma HLS PIPELINE II=1
        if (step >= total) break;

        int w = loser[0];

        // Read current element of winner (ARRAY_PARTITION makes this
        // a single-cycle register read, not a BRAM access)
        int cur_val;
        switch (w) {
            case 0: cur_val = local0[pos[0]]; break;
            case 1: cur_val = local1[pos[1]]; break;
            case 2: cur_val = local2[pos[2]]; break;
            default: cur_val = local3[pos[3]]; break;
        }

        output[step] = cur_val;
        pos[w]++;

      
        int cur  = w;
        int node = (K + w) >> 1;

ADJUST_LOOP: for (int lvl = 0; lvl < LOG_K; lvl++) {
#pragma HLS UNROLL
            if (node < 1) break;

            int l = loser[node];
            int lval, cval;

            switch (l) {
                case 0: lval = local0[pos[0]]; break;
                case 1: lval = local1[pos[1]]; break;
                case 2: lval = local2[pos[2]]; break;
                default: lval = local3[pos[3]]; break;
            }
            switch (cur) {
                case 0: cval = local0[pos[0]]; break;
                case 1: cval = local1[pos[1]]; break;
                case 2: cval = local2[pos[2]]; break;
                default: cval = local3[pos[3]]; break;
            }

            if (cval > lval) {
                int tmp     = loser[node];
                loser[node] = cur;
                cur         = tmp;
            }
            node >>= 1;
        }
        loser[0] = cur;
    }

    *out_size = total;
}