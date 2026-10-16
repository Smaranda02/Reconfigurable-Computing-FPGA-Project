// Identical test cases as naive testbench - ensures both are tested fairly.

#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <cassert>
#include "merge_sort_optimized.hpp"

static void make_sorted_run(int* buf, int n, int pad_val, std::mt19937& rng) {
    for (int i = 0; i < n; i++)
        buf[i] = rng() % 100000;
    std::sort(buf, buf + n);
    // pad rest with INF so merge loop never reads garbage
    for (int i = n; i < MAX_N; i++)
        buf[i] = pad_val;
}

static bool verify(int* out, int out_sz,
                   int* r0, int s0, int* r1, int s1,
                   int* r2, int s2, int* r3, int s3)
{
    std::vector<int> ref;
    for (int i = 0; i < s0; i++) ref.push_back(r0[i]);
    for (int i = 0; i < s1; i++) ref.push_back(r1[i]);
    for (int i = 0; i < s2; i++) ref.push_back(r2[i]);
    for (int i = 0; i < s3; i++) ref.push_back(r3[i]);
    std::sort(ref.begin(), ref.end());

    if (out_sz != (int)ref.size()) {
        std::cerr << "SIZE MISMATCH: got " << out_sz
                  << " expected " << ref.size() << "\n";
        return false;
    }
    for (int i = 0; i < out_sz; i++) {
        if (out[i] != ref[i]) {
            std::cerr << "MISMATCH at [" << i << "]: got "
                      << out[i] << " expected " << ref[i] << "\n";
            return false;
        }
    }
    return true;
}

int main() {
    std::mt19937 rng(42);
    int pass = 0, fail = 0;

    struct TC { int s0, s1, s2, s3; const char* label; };
    TC tests[] = {
        {   5,   5,   5,   5, "equal small (20 total)"      },
        {  10,   1,   7,   2, "unequal small (20 total)"    },
        { 100, 100, 100, 100, "medium equal (400 total)"    },
        { 200,  50, 300,  10, "medium unequal (560 total)"  },
        {1000,1000,1000,1000, "large equal (4000 total)"    },
        {   1,   1,   1,   1, "single element per run"      },
        {   0,   5,   0,   3, "some empty runs"             },
        {4096,4096,4096,4096, "max size (16384 total)"      },  // stress test
    };

    for (auto& t : tests) {
        static int r0[MAX_N], r1[MAX_N], r2[MAX_N], r3[MAX_N];
        static int out[K * MAX_N];
        int sizes[K] = {t.s0, t.s1, t.s2, t.s3};
        int out_size  = 0;

        make_sorted_run(r0, t.s0, INF_VAL, rng);
        make_sorted_run(r1, t.s1, INF_VAL, rng);
        make_sorted_run(r2, t.s2, INF_VAL, rng);
        make_sorted_run(r3, t.s3, INF_VAL, rng);

        merge_sort_optimized(r0, r1, r2, r3, sizes, out, &out_size);

        bool ok = verify(out, out_size,
                         r0, t.s0, r1, t.s1,
                         r2, t.s2, r3, t.s3);
        std::cout << "[" << (ok ? "PASS" : "FAIL") << "] " << t.label << "\n";
        if (ok) pass++; else fail++;

        if (t.s0 + t.s1 + t.s2 + t.s3 <= 20) {
            std::cout << "  output: ";
            for (int i = 0; i < out_size; i++)
                std::cout << out[i] << " ";
            std::cout << "\n";
        }
    }

    std::cout << "\n=== " << pass << " passed, " << fail << " failed ===\n";
    return (fail == 0) ? 0 : 1;
}