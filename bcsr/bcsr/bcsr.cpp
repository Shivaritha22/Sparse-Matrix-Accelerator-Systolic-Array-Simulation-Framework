#include <iostream>
#include <fstream>
using namespace std;

#define block_size 8
#define values_per_block (block_size*block_size)
#define MAX_ROWS 64
#define MAX_COLS 64
#define MAX_NNZ 4096

#define BR   (MAX_ROWS/block_size)   
#define BC   (MAX_ROWS/block_size) 
#define MAX_BLOCKS (BR*BC) 

// #define block_size 2
// #define MAX_ROWS 4
// #define MAX_COLS 4
// #define MAX_NNZ 16

void bcsr_format(
    const float A[MAX_ROWS][MAX_COLS],
    int rows, int cols,
    int row_ptr[MAX_ROWS],   
    int col[MAX_NNZ],        
    float val[MAX_NNZ],      
    int *size_blocks)        
{
    static float A_BRAM[MAX_ROWS][MAX_COLS];
    static int col_BRAM[MAX_NNZ];
    static float val_BRAM[MAX_NNZ];
    static int row_BRAM[MAX_NNZ];
    row_BRAM[0]=0;

    // copy matrix to BRAM
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            A_BRAM[r][c] = A[r][c];

    int blocks=rows/block_size;

    for (int out_row = 0; out_row < blocks; out_row++) {
    // remember starting point
    int row_start_blocks = *size_blocks;
    for (int out_col = 0; out_col < blocks; out_col++) {
        bool has_nnz = false;
        for (int in_row = 0; in_row < block_size; in_row++) {
            for (int in_col = 0; in_col < block_size; in_col++) {
                int r = out_row * block_size + in_row;
                int c = out_col * block_size + in_col;
                if (A_BRAM[r][c] != 0.0f) {
                    has_nnz = true;
                    break;
                }
            }
            if (has_nnz) break;
        }
        if (has_nnz) {
            col_BRAM[*size_blocks] = out_col * block_size; 
            int base = (*size_blocks) * block_size * block_size;
            for (int in_row = 0; in_row < block_size; in_row++) {
                for (int in_col = 0; in_col < block_size; in_col++) {
                    int r = out_row * block_size + in_row;
                    int c = out_col * block_size + in_col;
                    float v = 0.0f;
                    if (r < rows && c < cols) v = A_BRAM[r][c];
                    val_BRAM[base + in_row * block_size + in_col] = v;
                }
            }
            (*size_blocks)++;
        }
    }
    row_BRAM[out_row + 1] = *size_blocks;  

    }
    for (int i = 0; i < *size_blocks; ++i)
    col[i] = col_BRAM[i];

    int block_elems = block_size * block_size;
    for (int i = 0; i < (*size_blocks) * block_elems; ++i)
        val[i] = val_BRAM[i];

    for (int i = 0; i <= blocks; ++i)
        row_ptr[i] = row_BRAM[i];
}

void decompress_bcsr(
    const int   row_ptr_start_at_1[MAX_ROWS], 
    const int   col[MAX_NNZ],                 
    const float val[MAX_NNZ],                 
    int         size_blocks,                  
    float       A_out[MAX_ROWS][MAX_COLS])
{
    
    int   B0[BR] = {0}; 
    int   B1[MAX_BLOCKS] = {0};
    float BVAL[values_per_block][MAX_BLOCKS] = {0}; 

    // load B0
    for (int i = 0; i < BR; ++i) {
        B0[i] = row_ptr_start_at_1[i];
    }

    int nnz_blocks = size_blocks;
    if (nnz_blocks > MAX_BLOCKS) nnz_blocks = MAX_BLOCKS;

    // load block columns
    for (int k = 0; k < nnz_blocks; ++k) {
        B1[k] = col[k];
    }

    // load block values
    for (int k = 0; k < nnz_blocks; ++k) {
        int base = k * values_per_block;
        for (int t = 0; t < values_per_block; ++t) {
            BVAL[t][k] = val[base + t];
        }
    }

    for (int r = 0; r < MAX_ROWS; ++r)
        for (int c = 0; c < MAX_COLS; ++c)
            A_out[r][c] = 0.0f;

    // one block per cycle while diff>0 
    //cycles: nnz_blocks + 2*BR - 1
    int prev_off = 0;
    int blk_ptr  = 0;

    for (int br = 0; br < BR; ++br) {
        // READ B0
        int off  = B0[br];

        // COMPUTE diff
        int diff = off - prev_off;
        prev_off = off;

        // OFFLOAD diff blocks (one per cycle)
        while (diff > 0 && blk_ptr < nnz_blocks) {
            
            int bc_raw = B1[blk_ptr];
            int bc     = (bc_raw >= BC) ? (bc_raw / block_size) : bc_raw; 

            if (bc >= 0 && bc < BC) {
                int r0 = br * block_size;
                int c0 = bc * block_size;

                // write the block
                int t = 0;
                for (int ir = 0; ir < block_size; ++ir) {
                    for (int ic = 0; ic < block_size; ++ic) {
                        A_out[r0 + ir][c0 + ic] = BVAL[t][blk_ptr];
                        ++t; 
                    }
                }
            }

            // one block reconstructed per cycle
            ++blk_ptr;  
            --diff;
        }
    }

    
}
