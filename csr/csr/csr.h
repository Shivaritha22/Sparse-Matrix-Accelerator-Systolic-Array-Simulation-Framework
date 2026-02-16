#ifndef CSR_FORMAT_H
#define CSR_FORMAT_H

#define MAX_ROWS 64
#define MAX_COLS 64
#define MAX_NNZ  4096

void csr_format(
    const float A[MAX_ROWS][MAX_COLS],
    int rows,
    int cols,
    int offset[MAX_ROWS],
    int col[MAX_NNZ],
    float val[MAX_NNZ],
    int *size
);

void csr_decompression(
    float A[MAX_ROWS][MAX_COLS],
    int rows, int cols,
    int offset[MAX_ROWS],
    int col[MAX_NNZ],
    float val[MAX_NNZ],
    int *size
);

#endif
