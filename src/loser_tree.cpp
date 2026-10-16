//Streaming merge sort that builds a tournament tree (loser tree) to merge k sorted runs in O(n log k) time. 
// Suited for external/streaming scenarios where data arrives in chunks. The tree maintains the current minimum across all runs, 
// outputting sorted elements incrementally.


#include <iostream>
#include <climits>

static const int INF = INT_MAX;

struct Run {
    int* data;
    int  size;
    int  pos;

    int peek() const { return (pos < size) ? data[pos] : INF; }
    int pop()        { return (pos < size) ? data[pos++] : INF; }
};


//  WINNER TREE
struct WinnerTree {
    int  k;
    int* wtree;   // 2k, 1-based. wtree[1]=overall winner run index

    int val(Run* runs, int r) { return (r < k) ? runs[r].peek() : INF; }

    void build(Run* runs, int numRuns) {
        k = numRuns;
        wtree = new int[2 * k]();
        for (int i = 0; i < k; i++) wtree[k + i] = i;
        for (int i = k - 1; i >= 1; i--) {
            int L = wtree[2*i], R = wtree[2*i+1];
            wtree[i] = (val(runs,L) <= val(runs,R)) ? L : R;
        }
    }

    void update(Run* runs, int r) {
        int pos = k + r;
        wtree[pos] = r;
        pos >>= 1;
        while (pos >= 1) {
            int L = wtree[2*pos], R = wtree[2*pos+1];
            wtree[pos] = (val(runs,L) <= val(runs,R)) ? L : R;
            pos >>= 1;
        }
    }

    int getWinner() const { return wtree[1]; }
    ~WinnerTree() { delete[] wtree; }
};


//  LOSER TREE  (built from winner tree)
struct LoserTree {
    int  k;
    int* loser;   // loser[0] = overall winner run index
                  // loser[1..k-1] = loser at each internal node

    int val(Run* runs, int r) { return (r < k) ? runs[r].peek() : INF; }

    void build(Run* runs, int numRuns) {
        k = numRuns;
        loser = new int[k];

        // build a winner tree in a temp array to get match results
        int* wtree = new int[2 * k];
        for (int i = 0; i < k; i++) wtree[k + i] = i;

        // bottom-up - at each internal node, record winner & loser
        for (int i = k - 1; i >= 1; i--) {
            int L = wtree[2*i], R = wtree[2*i+1];
            if (val(runs, L) <= val(runs, R)) {
                wtree[i] = L;   // L wins, propagate up
                loser[i] = R;   // R loses, stays at node i
            } else {
                wtree[i] = R;
                loser[i] = L;
            }
        }
        // The winner of the root match is the overall winner
        loser[0] = wtree[1];

        delete[] wtree;
    }

    // After popping from run r, walk up and replay matches using stored losers
    void adjust(Run* runs, int r) {
        int cur  = r;
        int node = (k + r) >> 1;   // parent of run r's leaf

        while (node >= 1) {
            if (val(runs, cur) > val(runs, loser[node])) {
                // cur loses this match - store it, promote old loser
                std::swap(cur, loser[node]);
            }
            node >>= 1;
        }
        loser[0] = cur;   // new overall winner
    }

    int getWinner() const { return loser[0]; }
    ~LoserTree() { delete[] loser; }
};


void printArray(const char* label, int* arr, int n) {
    std::cout << label << ": [ ";
    for (int i = 0; i < n; i++)
        std::cout << arr[i] << (i+1<n ? ", " : " ");
    std::cout << "]\n";
}

int main() {
    int r0[] = {1, 2, 3};
    int r1[] = {3, 5, 9, 12};
    int r2[] = {7, 8, 10, 11, 13};
    int r3[] = {4, 6, 9};
    int r4[] = {3, 5, 7, 9, 10};

    const int k = 5;
    int* ptrs[]  = {r0, r1, r2, r3, r4};
    int  sizes[] = {3, 4, 5, 3, 5};
    const int totalSize = 20;

    Run runs[k];
    auto resetRuns = [&]() {
        for (int i = 0; i < k; i++) runs[i] = {ptrs[i], sizes[i], 0};
    };

    std::cout << "=== Input Runs ===\n";
    for (int i = 0; i < k; i++) {
        char label[16]; std::snprintf(label, 16, "Run %d", i);
        printArray(label, ptrs[i], sizes[i]);
    }

    int output[totalSize], idx;

    // Winner Tree merge
    resetRuns(); idx = 0;
    {
        WinnerTree wt; wt.build(runs, k);
        while (wt.val(runs, wt.getWinner()) != INF) {
            int w = wt.getWinner();
            output[idx++] = runs[w].pop();
            wt.update(runs, w);
        }
    }
    std::cout << "\n=== Winner Tree Merge ===\n";
    printArray("Sorted", output, idx);

    // Loser Tree merge
    resetRuns(); idx = 0;
    {
        LoserTree lt; lt.build(runs, k);
        while (true) {
            int w = lt.getWinner();
            if (w >= k || lt.val(runs, w) == INF) break;
            output[idx++] = runs[w].pop();
            lt.adjust(runs, w);
        }
    }
    std::cout << "\n=== Loser Tree Merge ===\n";
    printArray("Sorted", output, idx);

    return 0;
}