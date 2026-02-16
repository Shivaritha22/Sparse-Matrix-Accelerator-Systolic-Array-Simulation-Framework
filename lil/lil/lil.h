#ifndef LIL_H
#define LIL_H

#include <vector>
using namespace std;

#define SIZE 64
#define MAX_ROWS 64
#define MAX_COLS 64


void lil_format(const vector<vector<int>>& A,
                          vector<vector<int>>& row_out,
                          vector<vector<int>>& val_out);

int lil_decompress(const vector<vector<int>>& row_in,
                              const vector<vector<int>>& val_in,
                              vector<vector<int>>& A,
                              vector<vector<int>>& BRAM);

#endif 
