
# Cache Prefetcher Implementation & Analysis for ChampSim

The speed and efficiency of CPUs are frequently limited by memory latency—the delay in fetching data from main memory. This project explores **hardware prefetching**, a technique that predicts future memory accesses and loads data into the cache ahead of time to get a better access time.

Two distinct prefetcher designs were implemented and evaluated within the [ChampSim](https://github.com/ChampSim/ChampSim "null") architectural simulator: a simple and efficient **Adaptive Next-N-Line Prefetcher** and a more complex, pattern-based **Global History Buffer (GHB) Prefetcher**. The goal was to analyze the performance trade-offs between these different strategies across various application workloads.

## Key Features

This project contains the source code for two C++ based prefetchers:

### 1. Adaptive Next-N-Line Prefetcher

A sequential prefetcher that identifies and fetches data for simple patterns.

- **Core Logic**: When there is a cache miss on address `A`, it prefetches the next `N` consecutive cache lines (`A+1`, `A+2`, ..., `A+N`).
- **Adaptive Degree**: The prefetch degree `N` is not fixed. It dynamically adjusts based on the measured accuracy of recent prefetches.
- **Feedback Loop**:
  - If accuracy is high (e.g. `>60%`), it increases `N` to be more aggressive.
  - If accuracy is low (e.g. `<20%`), it decreases `N` to be more conservative, saving memory bandwidth.

### 2. Global History Buffer (GHB) Prefetcher

A sophisticated, pattern-based prefetcher designed to detect complex, non-sequential memory access patterns.

- **Instruction Correlation**: It correlates memory accesses with the instruction pointer (IP) that generated them using an **Index Table (IT)**.
- **History Tracking**: A **Global History Buffer (GHB)** stores recent memory accesses, creating a threaded history for each IP.
- **Pattern Detection**: By analyzing the address differences (deltas) in an instruction's access history, it can detect complex stride patterns (e.g., `+1, +8, +1, +8, ...`).
- **Targeted Prefetching**: Once a stable pattern is found, it issues targeted prefetches based on the detected stride, enabling it to handle irregular but predictable access patterns that sequential prefetchers would miss.

## How to Replicate

To build and run these prefetchers, copy the folder of the prefetcher you would like to test to your local ChampSim prefetcher directory and follow the steps mentioned in the official GitHub page of ChampSim.

## Analysis and Results

- The prefetchers were evaluated using multiple trace files provided on the official GitHub page of ChampSim. The primary metrics for comparison were the **L2 Cache Average Miss Latency** and **Prefetcher Accuracy**.
- We have added screenshots of adaptive_next_n_line and ghb_prefetcher compared with no_prefetcher using same trace file and same parameters in the `results` folder.

### Performance Summary

| Configuration            | L2C Average Miss Latency | Prefetcher Accuracy |
|--------------------------|---------------------------|----------------------|
| No Prefetcher (Baseline) | 169.7 cycles              | N/A                  |
| Adaptive Next-N-Line     | 168.5 cycles              | 8.1%                 |
| GHB Prefetcher           | **168.2 cycles**          | **11.7%**            |


### Key Findings

- **Performance Improvement**: Both prefetchers reduced the average L2 cache miss latency compared to no prefetcher, directly contributing to a potential increase in overall IPC. The **GHB Prefetcher** provided the best performance, reducing latency by **1.5 cycles**, slightly more than the Adaptive prefetcher's reduction of **1.2 cycles**.
- **Prefetcher Accuracy**: The GHB prefetcher was not only more effective but also more accurate on this workload, with **11.7%** of its prefetches being useful, compared to **8.1%** for the simpler adaptive prefetcher. This indicates that the trace file used here contains complex or strided memory access patterns that the GHB's instruction-correlated history was able to identify and exploit.
- **Conclusion**: For this workload, the higher complexity and hardware overhead of the GHB prefetcher were justified, as it delivered both a greater reduction in memory latency and higher accuracy. This demonstrates its strength in handling workloads that are not purely sequential.

## Skills Demonstrated

- **Computer Architecture:** Caches, Memory Hierarchy, Hardware Prefetching, Performance Bottlenecks.
- **C++ Programming:** Advanced data structures, object-oriented design, and implementation within a large, existing codebase.
- **Performance Analysis:** Benchmarking and interpreting performance metrics (IPC, Cache Miss Rates, Accuracy).
- **Problem Solving:** Designing and implementing complex algorithms to solve real-world system performance challenges.
