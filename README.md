# Sparse-Matrix-Accelerator-Systolic-Array-Simulation-Framework
Built C++ simulators for stationary and non-stationary systolic arrays, modeling dataflow, memory access, and cycle-accurate latency for matrix multiplication. Implemented CSR, COO, and BCSR formats to study storage–latency trade-offs. Benchmarked 20 sparse matrices and validated performance using roofline-based throughput analysis.


# Sparse Matrix Accelerator & Systolic Array Simulation Framework

Cycle-accurate C++ simulation framework for evaluating sparse matrix formats on tiled systolic-array style accelerators.

This project models dataflow, memory access behavior, metadata overhead, and decompression latency across CSR, BCSR, COO, and LIL formats using BRAM-style buffering and deterministic cycle estimation formulas.

---

## 🔍 Project Motivation

Modern ML accelerators and matrix engines rely heavily on structured data movement and memory efficiency. Sparse formats reduce storage but introduce decompression and metadata overhead.

This framework was built to answer:

- How do sparse formats trade off storage vs latency?
- When does metadata dominate?
- How does block-based compression affect throughput?
- How do these decisions map to systolic-array-style compute pipelines?

The simulator provides controlled, tile-level modeling to study these trade-offs.

---

## 🏗 System Architecture

The simulator models a:

- **1024 × 1024 matrix multiplication**
- Partitioned into **64 × 64 tiles**
- 16 × 16 tile grid
- Per-tile compression → decompression → reconstruction
- Full-matrix verification

Each tile:
1. Is compressed into a selected sparse format  
2. Is decompressed using a BRAM-buffered pipeline  
3. Has cycle-level latency computed  
4. Is verified against the original dense tile  

Cycle estimation reflects structured memory reads and emission constraints.

---

## 📦 Supported Sparse Formats

### CSR (Compressed Sparse Row)

Metadata:
- Row pointers
- Column indices

Cycle model per tile:
```cpp
cycles = 64 + 64 + nnz - 1
```

Characteristics:
- Linear scaling with nonzeros
- High row pointer overhead at extreme sparsity
- Predictable latency growth

---

### BCSR (Block CSR, block size = 8)

Metadata:
- Block row pointers
- Block column indices

Cycle model per tile:
```cpp
cycles = nnz_blocks + 2*BR - 1
```

Characteristics:
- Stores full 8×8 payload per nonzero block
- Reduces metadata significantly in clustered sparsity
- Trades storage padding for compute regularity

---

### COO (Coordinate Format)

Metadata:
- Row index per nonzero
- Column index per nonzero

Cycle model:
```cpp
cycles = nnz
```

Characteristics:
- Minimal structural overhead
- Best at ultra-low density
- Latency strictly linear in nonzeros

---

### LIL (List of Lists)

Metadata:
- Row-structured storage

Cycle model:
```cpp
cycles = number_of_distinct_nonzero_rows
```

Characteristics:
- Stable metadata cost
- Latency dependent on row activation
- Useful for row-clustered sparsity patterns

---

## 🧠 BRAM-Based Architectural Modeling

All decompression paths simulate:

- On-chip BRAM buffering
- Burst tile loads
- Local row/column scans
- Controlled emission into compute pipeline
- Sequentialization to avoid banking hazards

The simulator explicitly models:
- Shared memory hazards
- Banking constraints
- Emission rate limits
- Deterministic cycle accumulation

This allows reproducible latency comparisons across formats.

---

## 📊 Datasets

### Synthetic Matrices

Generated using probabilistic sampling across densities:

```
0.0001 → 0.7
```

Each matrix:
- Size: 1024 × 1024
- Nonzero value = 1
- Controlled density variation

---

### SuiteSparse Collection (Cropped)

Real-world sparse matrices (cropped to 1024 × 1024):

- Diagonal dominant
- Block clustered
- Asymmetric
- Structured sparsity

These provide realistic access patterns beyond synthetic randomness.

---

## ⚙ Benchmark Configuration

- 20 matrices evaluated
- 64 × 64 tiling
- Per-tile compression and decompression
- Global reconstruction validation
- Log-scale metadata ratio tracking
- Log-scale latency measurement
- Roofline-based throughput validation

---

## 📈 Format Comparison Summary

| Format | Metadata Overhead | Latency Scaling | Best Regime |
|---------|------------------|----------------|-------------|
| CSR     | Moderate–High at low density | Linear with nnz + row offsets | Predictable sparse |
| BCSR    | Reduced metadata, padded blocks | Stable across moderate density | Clustered sparsity |
| COO     | Minimal structural metadata | Linear with nnz | Ultra-sparse |
| LIL     | Stable row metadata | Scales with active rows | Row-structured |

---

## 📊 Observed Trends

- BCSR achieved significant metadata reduction in moderate-density regimes.
- CSR overhead dominates at ultra-low densities due to row pointer cost.
- COO performs best under extreme sparsity.
- LIL benefits row-clustered matrices.
- Roofline validation confirms the workload is bandwidth-dominated in sparse regimes.

---

## 📉 Storage–Latency Trade-Off Insight

| Density Range | Recommended Format | Reasoning |
|---------------|-------------------|-----------|
| 0.0001–0.01   | COO               | Lowest metadata footprint |
| 0.01–0.2      | BCSR              | Balanced block efficiency |
| >0.2          | CSR               | Predictable scaling |
| Row-clustered | LIL               | Reduced row activation overhead |

---

## ▶ How to Build and Run

Compile:
```bash
g++ <format_main>.cpp -o run
```

Execute:
```bash
./run
```

Execution performs:
- Tiling
- Compression
- Decompression
- Cycle logging
- Full reconstruction verification

---

## 🏗 Engineering Focus

This project emphasizes:

- Sparse dataflow modeling
- Storage–latency trade-off analysis
- Block-based compression effects
- On-chip buffering strategies
- Cycle-accurate decompression modeling
- Roofline-based performance reasoning

It is designed as a research-style simulator for exploring sparse accelerator and ML dataflow design decisions.

---

## 👤 Author

Shivaritha Sakthi Rengasamy
