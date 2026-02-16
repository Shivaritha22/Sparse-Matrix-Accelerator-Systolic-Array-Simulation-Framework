#ifndef CSR_FORMAT_H
#define CSR_FORMAT_H

#define MAX_ROWS 64
#define MAX_COLS 64
#define MAX_NNZ 4096

void coo_format(
    const float A[MAX_ROWS][MAX_COLS], int tuple_A[MAX_NNZ][3], int *size);

void coo_decompression(
    float A[MAX_ROWS][MAX_COLS],
    int tuple_A[MAX_NNZ][3],
    int *size
);
#endif
