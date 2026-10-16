#include <iostream>
#include <climits>
#include <chrono>
#include <vector>
#include <algorithm>
#include <random>
#include <iomanip>

static const int INF = INT_MAX;

struct Run {
    int* data; 
    int size; 
    int pos;
    
    int peek() const { return (pos < size) ? data[pos] : INF; }
    int pop()        { return (pos < size) ? data[pos++] : INF; }
};

struct WinnerTree {
    int k; 
    int* wtree;
    
    int val(Run* runs, int r) { return (r < k) ? runs[r].peek() : INF; }
    
    void build(Run* runs, int numRuns) {
        k = numRuns; 
        wtree = new int[2*k]();
        
        for (int i = 0; i < k; i++) 
            wtree[k+i] = i;
            
        for (int i = k-1; i >= 1; i--) {
            int L = wtree[2*i], R = wtree[2*i+1];
            wtree[i] = (val(runs, L) <= val(runs, R)) ? L : R;
        }
    }
    
    void update(Run* runs, int r) {
        int pos = k + r; 
        wtree[pos] = r; 
        pos >>= 1;
        
        while (pos >= 1) {
            int L = wtree[2*pos], R = wtree[2*pos+1];
            wtree[pos] = (val(runs, L) <= val(runs, R)) ? L : R;
            pos >>= 1;
        }
    }
    
    int getWinner() const { return wtree[1]; }
    ~WinnerTree() { delete[] wtree; }
};

struct LoserTree {
    int k; 
    int* loser;
    
    int val(Run* runs, int r) { return (r < k) ? runs[r].peek() : INF; }
    
    void build(Run* runs, int numRuns) {
        k = numRuns; 
        loser = new int[k];
        int* wtree = new int[2*k];
        
        for (int i = 0; i < k; i++) 
            wtree[k+i] = i;
            
        for (int i = k-1; i >= 1; i--) {
            int L = wtree[2*i], R = wtree[2*i+1];
            if (val(runs, L) <= val(runs, R)) {
                wtree[i] = L;
                loser[i] = R;
            } else {
                wtree[i] = R;
                loser[i] = L;
            }
        }
        
        loser[0] = wtree[1]; 
        delete[] wtree;
    }
    
    void adjust(Run* runs, int r) {
        int cur = r, node = (k + r) >> 1;
        
        while (node >= 1) {
            if (val(runs, cur) > val(runs, loser[node])) 
                std::swap(cur, loser[node]);
            node >>= 1;
        }
        
        loser[0] = cur;
    }
    
    int getWinner() const { return loser[0]; }
    ~LoserTree() { delete[] loser; }
};

// generate k sorted runs with total n elements
std::vector<std::vector<int>> make_runs(int n, int k, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::vector<std::vector<int>> runs(k);
    
    for (int i = 0; i < n; i++) 
        runs[i % k].push_back(rng() % 1000000);
    
    for (auto& r : runs) 
        std::sort(r.begin(), r.end());
    
    return runs;
}

using us = std::chrono::microseconds;
using Clock = std::chrono::high_resolution_clock;

double measure_winner(int n, int k, int reps = 5) {
    auto data = make_runs(n, k);
    std::vector<Run> runs(k);
    std::vector<int> out(n);
    
    double total = 0;
    
    for (int rep = 0; rep < reps; rep++) {
        for (int i = 0; i < k; i++) 
            runs[i] = {data[i].data(), (int)data[i].size(), 0};
        
        int idx = 0;
        auto t0 = Clock::now();
        
        WinnerTree wt; 
        wt.build(runs.data(), k);
        
        while (wt.val(runs.data(), wt.getWinner()) != INF) {
            int w = wt.getWinner();
            out[idx++] = runs[w].pop();
            wt.update(runs.data(), w);
        }
        
        auto t1 = Clock::now();
        total += std::chrono::duration<double, std::micro>(t1 - t0).count();
    }
    
    return total / reps;
}

double measure_loser(int n, int k, int reps = 5) {
    auto data = make_runs(n, k);
    std::vector<Run> runs(k);
    std::vector<int> out(n);
    
    double total = 0;
    
    for (int rep = 0; rep < reps; rep++) {
        for (int i = 0; i < k; i++) 
            runs[i] = {data[i].data(), (int)data[i].size(), 0};
        
        int idx = 0;
        auto t0 = Clock::now();
        
        LoserTree lt; 
        lt.build(runs.data(), k);
        
        while (true) {
            int w = lt.getWinner();
            if (w >= k || lt.val(runs.data(), w) == INF) 
                break;
            out[idx++] = runs[w].pop();
            lt.adjust(runs.data(), w);
        }
        
        auto t1 = Clock::now();
        total += std::chrono::duration<double, std::micro>(t1 - t0).count();
    }
    
    return total / reps;
}

int main() {
    // vary N, fixed k=4
    std::vector<int> sizes = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    int k = 4;

    std::cout << "\n--- Vary N (k=" << k << " fixed) ---\n";
    std::cout << std::left
              << std::setw(12) << "N"
              << std::setw(18) << "WinnerTree (us)"
              << std::setw(18) << "LoserTree (us)"
              << std::setw(16) << "us/elem (WT)"
              << std::setw(16) << "us/elem (LT)"
              << "\n" << std::string(80, '-') << "\n";

    for (int n : sizes) {
        double wt = measure_winner(n, k);
        double lt = measure_loser(n, k);
        
        std::cout << std::setw(12) << n
                  << std::setw(18) << std::fixed << std::setprecision(2) << wt
                  << std::setw(18) << lt
                  << std::setw(16) << std::setprecision(4) << wt / n
                  << std::setw(16) << lt / n
                  << "\n";
    }

    // vary k, fixed n=10000
    int n = 10000;
    std::vector<int> ks = {2, 4, 8, 16, 32, 64};

    std::cout << "\n--- Vary k (n=" << n << " fixed) ---\n";
    std::cout << std::left
              << std::setw(8)  << "k"
              << std::setw(18) << "WinnerTree (us)"
              << std::setw(18) << "LoserTree (us)"
              << "\n" << std::string(44, '-') << "\n";

    for (int ki : ks) {
        double wt = measure_winner(n, ki);
        double lt = measure_loser(n, ki);
        
        std::cout << std::setw(8)  << ki
                  << std::setw(18) << std::fixed << std::setprecision(2) << wt
                  << std::setw(18) << lt
                  << "\n";
    }

    return 0;
}


