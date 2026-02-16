#include "lil.h"
using namespace std;

void lil_format(const vector<vector<int>>& A,
                          vector<vector<int>>& row_out,
                          vector<vector<int>>& val_out) {
    row_out.assign(SIZE, {});
    val_out.assign(SIZE, {});
    for (int j = 0; j < SIZE; ++j) {
        for (int i = 0; i < SIZE; ++i) {
            int v = A[i][j];
            if (v != 0) {
                row_out[j].push_back(i);
                val_out[j].push_back(v);
            }
        }
    }
}

int lil_decompress(const vector<vector<int>>& row_in,
                   const vector<vector<int>>& val_in,
                   vector<vector<int>>& A,
                   vector<vector<int>>& BRAM) {
    
    A.assign(SIZE, vector<int>(SIZE, 0));

    BRAM.assign(SIZE * 2, vector<int>(SIZE, 0));

    // load compressed columns into BRAM with padding
    for (int j = 0; j < SIZE; ++j) {
        const int col_nnz = static_cast<int>(row_in[j].size()); 
        for (int k = 0; k < SIZE; ++k) {
            if (k < col_nnz) {
                BRAM[k][j] = row_in[j][k];
                BRAM[k + SIZE][j] = val_in[j][k];
            } else {
                // pad index
                // pad value
                BRAM[k][j] = -1;             
                BRAM[k + SIZE][j] =  0;             
            }
        }
    }

    // // reconstruct dense A from BRAM
    // for (int j = 0; j < SIZE; ++j) {
    //     for (int k = 0; k < SIZE; ++k) {
    //         int r = BRAM[k][j];
    //         if (r != -1) {
    //             A[r][j] = BRAM[k + SIZE][j];
    //         }
    //     }
    // }

    // // total cycles = number of non-zero rows
    // int cycles = 0;
    // for (int i = 0; i < SIZE; ++i) {
    //     bool nz = false;
    //     for (int j = 0; j < SIZE; ++j) {
    //         if (A[i][j] != 0) { nz = true; break; }
    //     }
    //     if (nz) ++cycles;
    // }
    // return cycles;

    vector<int> nz_rows;
nz_rows.reserve(SIZE);
vector<char> seen(SIZE, 0);

for (int j = 0; j < SIZE; ++j) {
    for (int k = 0; k < SIZE; ++k) {
        int r = BRAM[k][j];
        if (r == -1) break;                 // past real entries in this column
        int v = BRAM[k + SIZE][j];
        if (v != 0 && !seen[r]) {           // first time we see a real value in row r
            seen[r] = 1;
            nz_rows.push_back(r);
        }
    }
}

// 2) Cycles = number of non-zero rows (by construction).
int cycles = static_cast<int>(nz_rows.size());

// 3) Exactly `cycles` iterations; each "cycle" reconstructs one full row.
for (int t = 0; t < cycles; ++t) {
    int r_target = nz_rows[t];
    // fill A[r_target][*] by scanning all columns for this row's entries
    for (int j = 0; j < SIZE; ++j) {
        // find r_target in column j
        for (int k = 0; k < SIZE; ++k) {
            int r = BRAM[k][j];
            if (r == -1) break;             // no more entries in this column
            if (r == r_target) {
                A[r_target][j] = BRAM[k + SIZE][j];
                break;                       // next column
            }
        }
    }
}

return cycles;
}