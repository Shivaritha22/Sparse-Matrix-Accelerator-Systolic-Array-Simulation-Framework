#ifndef BCSR_FORMAT_H
#define BCSR_FORMAT_H


#define block_size 8
#define MAX_ROWS   64
#define MAX_COLS   64
#define MAX_NNZ    4096

#define values_per_block (block_size*block_size)
#define BLOCK_SIZE       block_size
#define BR               (MAX_ROWS / BLOCK_SIZE)          
#define BC               (MAX_COLS / BLOCK_SIZE)          
#define VALS_PER_BLOCK   (BLOCK_SIZE * BLOCK_SIZE)        
#define MAX_BLOCKS       (BR * BC)                        


void bcsr_format(
    const float A[MAX_ROWS][MAX_COLS],
    int rows, int cols,
    int row_ptr[MAX_ROWS + 1],   
    int col[MAX_NNZ],            
    float val[MAX_NNZ],          
    int *size_blocks);           


void decompress_bcsr(
    const int   row_ptr_start_at_1[],            
    const int   col[],                           
    const float val[],                           
    int         size_blocks,                     
    float       A_out[MAX_ROWS][MAX_COLS]);      

#endif 