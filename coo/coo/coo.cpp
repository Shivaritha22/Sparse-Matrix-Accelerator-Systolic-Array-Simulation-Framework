#include <algorithm>
using namespace std;

#define MAX_ROWS 64
#define MAX_COLS 64
#define MAX_NNZ  4096

void coo_format(const float A[MAX_ROWS][MAX_COLS],
                int tuple_A[MAX_NNZ][3],
                int *size)
{
    // BRAM
    static float A_BRAM[MAX_ROWS][MAX_COLS];
    static int   tuple_A_BRAM[MAX_NNZ][3];   

    // burst read A -> BRAM
    for (int i = 0; i < MAX_ROWS; ++i)
        for (int j = 0; j < MAX_COLS; ++j)
            A_BRAM[i][j] = A[i][j];

    // build COO (row, col, val) into BRAM
    int nnz = 0;
    for (int i = 0; i < MAX_ROWS; ++i) {
        for (int j = 0; j < MAX_COLS; ++j) {
            float v = A_BRAM[i][j];
            if (v != 0.0f) {
                if (nnz >= MAX_NNZ) break;          
                tuple_A_BRAM[nnz][0] = i;           
                tuple_A_BRAM[nnz][1] = j;           
                tuple_A_BRAM[nnz][2] = (int)v;      
                ++nnz;                              
            }
        }
    }

    *size = nnz;

    // burst write BRAM
    for (int k = 0; k < nnz; ++k) {
        tuple_A[k][0] = tuple_A_BRAM[k][0];
        tuple_A[k][1] = tuple_A_BRAM[k][1];
        tuple_A[k][2] = tuple_A_BRAM[k][2];
    }
}

void coo_decompression(float A[MAX_ROWS][MAX_COLS],
                       int tuple_A[MAX_NNZ][3],
                       int *size)
{
    //total cycles = number of non-zero rows
    int total_cycles= *size; 
    for (int i = 0; i < *size; ++i) { 
        for (int j = 0; j < 3; ++j) { 
            
            //#pragma HLS PIPELINE II=1 
            int row = tuple_A[i][0]; 
            int col = tuple_A[i][1]; 
            float val = tuple_A[i][2]; 
            A[row][col] = val; 
        } 
    }
}
