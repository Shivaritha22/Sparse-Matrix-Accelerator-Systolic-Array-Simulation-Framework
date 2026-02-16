#include "csr.h"
#include <iostream>
using namespace std;

void csr_format(
    const float A[MAX_ROWS][MAX_COLS],
    int rows, int cols,
    int offset[MAX_ROWS],
    int col[MAX_NNZ],
    float val[MAX_NNZ],
    int *size)
{
   
    // BRAM
    static float A_BRAM[MAX_ROWS][MAX_COLS];
    static int   offset_BRAM[MAX_ROWS];
    static float val_BRAM[MAX_NNZ];
    static int   col_BRAM[MAX_NNZ];
    

    // copy matrix to BRAM
    // burst read A matrix
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            //#pragma HLS PIPELINE II=1
            A_BRAM[i][j] = A[i][j];
        }
    }

    // perform csr formatting on BRAM
    int nnz = 0;
    for (int i = 0; i < rows; ++i) {
        //#pragma HLS PIPELINE II=1
        int row_nnz = 0;

        for (int j = 0; j < cols; ++j) {
            //#pragma HLS PIPELINE II=1
            float value = A_BRAM[i][j];
            if (value != 0.0f) {
                // write into BRAM
                val_BRAM[nnz + row_nnz] = value;
                col_BRAM[nnz + row_nnz] = j;
                row_nnz++;
            }
        }

        offset_BRAM[i] = nnz + row_nnz;
        nnz += row_nnz;
    }

    for (int k = 0; k < nnz; ++k) {
        //#pragma HLS PIPELINE II=1
        val[k] = val_BRAM[k];
        col[k] = col_BRAM[k];
    }
    for (int r = 0; r < rows; ++r) {
        //#pragma HLS PIPELINE II=1
        offset[r] = offset_BRAM[r];
    }

    *size = nnz;
}

void csr_decompression(
    float A[MAX_ROWS][MAX_COLS],
    int rows, int cols,
    int offset[MAX_ROWS],
    int col[MAX_NNZ],
    float val[MAX_NNZ],
    int *size)
{
    int total_cycles, diff, diff_flag, load, loop, i, j, current_row, B0;
    // allocate time for decompression
    // total cycles = size[rows] + size[offset] + size[val] -1
    total_cycles = rows + rows + (*size);
    //cout << "\nTotal decompression cycles: "<<total_cycles<<endl;
    diff = -1;
    load = 1; i=0; j=-1; diff_flag=0; loop=0; current_row=0; B0=0;
    for (int cycle=0; cycle<=total_cycles; cycle++) {
        // write value to A matrix
        if (loop){
            //cout<<"\ndiff loop";
            if(diff){
                j++;
                A[i][col[j]] = val[j];
                diff--;
                if(diff==0) {
                    loop=0;
                    load=1;
                }
            }
            else{
                loop=0;
                load=1;
            }
        }
        
        // compute if row exists
        else if(diff_flag)
        {
            //cout<<"\ncompute";
            if(i == 0){
                diff = B0;
                diff_flag = 0;
                loop=1;
            }
            else{
                diff = B0-offset[i-1];
                if (diff){
                diff_flag = 0;
                loop=1;
                }
                else{
                    load=1;
                    diff_flag=0;
                }
            }
        }

        // load offset
        else if (load)
        {
           // cout<<"\nload";
            if(current_row<rows){
                i= current_row;
                B0=offset[i];
                current_row++;
                load=0;
                diff_flag=1;
            }
            else{
                if (!loop && !diff_flag && !load) break;
            }
            
        }
       
        //cout<<"\ncycle: "<<cycle<<endl;

    }


}
