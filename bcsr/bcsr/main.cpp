#include <iostream>
#include <fstream>
#include "bcsr.h"
using namespace std;

int main() {
    float A[MAX_ROWS][MAX_COLS] = {0};
    int rows = MAX_ROWS, cols = MAX_COLS;

    int fileNum;
    cout << "Enter matrix file number: ";
    cin >> fileNum;

    string matrixFile = "matrix" + to_string(fileNum);
    string compressedFile = "compressed" + to_string(fileNum);
    string decompressedFile = "decompressed" + to_string(fileNum);

    // read matrix from file
    ifstream fin(matrixFile + ".txt");
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            fin >> A[i][j];
    fin.close();

    
    int block_rows = rows / block_size;
    int row_ptr[MAX_ROWS] = {0};
    int col[MAX_NNZ] = {0};
    float val[MAX_NNZ] = {0};
    int size_blocks = 0;

    
    cout << "\nBCSR Format:"<<endl;
    bcsr_format(A, rows, cols, row_ptr, col, val, &size_blocks);
    
    ofstream fout(compressedFile + ".txt");
    if(fout.is_open()) {
        fout << "Column indices\n";
        for (int i = 0; i < size_blocks; ++i) fout << col[i] << " ";

        fout << "\nNon-zero block values\n";
        int block_elems = block_size * block_size;
        for (int i = 0; i < size_blocks * block_elems; ++i)
            fout << val[i] << ((i+1)%block_elems==0 ? "\n" : " ");

        fout << "\nRow offsets\n";
        for (int i = 1; i <= block_rows; ++i) fout << row_ptr[i] << " ";
        fout << endl;
        fout.close();
        cout << "Compression format generated" << endl; 
    } else {
        cerr << "Error opening file for writing" << endl;
    }

    
    float A_dec[MAX_ROWS][MAX_COLS] = {0};

    decompress_bcsr(&row_ptr[1], col, val, size_blocks, A_dec);

    cout << "\nBCSR Decompression:"<<endl;
    ofstream fout_dec(decompressedFile + ".txt");
    if (fout_dec.is_open()) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j)
                fout_dec << A_dec[i][j] << " ";
            fout_dec<<endl;
        }
        fout_dec.close();
        cout << "Decompressed matrix generated" << endl;
    } else {
        cerr << "Error opening file for writing" << endl;
        return 1;
    }

    
    // functionality check
    ifstream f1(matrixFile + ".txt"), f2(decompressedFile + ".txt");
    if (!f1.is_open() || !f2.is_open()) {
        cerr << "Error opening files" << endl;
        return 1;
    }

    string line1, line2;
    int row = 0;
    bool identical = true;

    cout << "\nFunctionality check:"<<endl;

    while (getline(f1, line1) && getline(f2, line2)) {
        row++;
        if (line1 != line2) {
            cout << "Difference found in row " << row << endl;
            identical = false;
        }
    }

    if ((getline(f1, line1) || getline(f2, line2))) {
        cout << "Files have different number of rows" << endl;
        identical = false;
    }

    if (identical) cout << "Files are identical" << endl;

    return 0;
}
