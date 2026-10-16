#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <cassert>
#include "merge_sort_naive.hpp"

// generate a sorted run of length n
static void make_sorted_run(int* buf, int n, std::mt19937& rng) {
    for (int i = 0; i < n; i++)
        buf[i] = rng() % 100000;
    std::sort(buf, buf + n);
}

// verify output is sorted and contains all input elements
static bool verify(int* out, int out_sz,
                   int* r0, int s0,
                   int* r1, int s1,
                   int* r2, int s2,
                   int* r3, int s3)
{
    // build reference by merging all runs and sorting
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
            std::cerr << "MISMATCH at index " << i
                      << ": got " << out[i]
                      << " expected " << ref[i] << "\n";
            return false;
        }
    }
    return true;
}

// Main testbench
int main() {
    std::mt19937 rng(42);
    int pass = 0, fail = 0;

    // Test cases: (s0, s1, s2, s3) 
    struct TC { int s0, s1, s2, s3; const char* label; };
    TC tests[] = {
        {  5,   5,   5,   5, "equal small (20 total)"       },
        { 10,   1,   7,   2, "unequal small (20 total)"     },
        {100, 100, 100, 100, "medium equal (400 total)"     },
        {200,  50, 300,  10, "medium unequal (560 total)"   },
        {1000,1000,1000,1000,"large equal (4000 total)"     },
        {  1,   1,   1,   1, "single element per run"       },
        {  0,   5,   0,   3, "some empty runs"              },  // sizes 0 = empty
    };

    for (auto& t : tests) {
        // allocate buffers (static for HLS compatibility)
        static int r0[MAX_N], r1[MAX_N], r2[MAX_N], r3[MAX_N];
        static int out[K * MAX_N];
        int sizes[K] = {t.s0, t.s1, t.s2, t.s3};
        int out_size = 0;

        // generate sorted runs
        if (t.s0 > 0) make_sorted_run(r0, t.s0, rng);
        if (t.s1 > 0) make_sorted_run(r1, t.s1, rng);
        if (t.s2 > 0) make_sorted_run(r2, t.s2, rng);
        if (t.s3 > 0) make_sorted_run(r3, t.s3, rng);

        // call DUT
        merge_sort_naive(r0, r1, r2, r3, sizes, out, &out_size);

        // verify
        bool ok = verify(out, out_size,
                         r0, t.s0, r1, t.s1,
                         r2, t.s2, r3, t.s3);
        std::cout << "[" << (ok ? "PASS" : "FAIL") << "] " << t.label << "\n";
        if (ok) pass++; else fail++;

        // print first few elements for visual check on small tests
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