#include <iostream>
#include <fstream>
#include "csr.h"
using namespace std;

int main() {
    // Example dense matrix
    float A[MAX_ROWS][MAX_COLS] = {0};
    int rows = MAX_ROWS, cols = MAX_COLS;

    // // Fill example values
    // /*
    // 1 2 0 0
    // 0 3 4 0
    // 0 0 5 6
    // 0 0 0 7
    // */
    // A[0][0] = 1;
    // A[0][2] = 2;
    // A[1][1] = 3;
    // A[2][0] = 4;
    // A[2][1] = 5;
    // A[2][2] = 6;
    // A[3][3] = 7;


    int fileNum;
    cout << "Enter matrix file number: ";
    cin >> fileNum;

    string matrixFile = "matrix" + to_string(fileNum);
    string compressedFile = "compressed" + to_string(fileNum);
    string decompressedFile = "decompressed" + to_string(fileNum);

    ifstream fin(matrixFile + ".txt");
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
           fin >> A[i][j];
    fin.close();

    // cout << "Original Matrix"<<endl;
    // for(int i = 0; i < rows; i++) {
    //     for(int j = 0; j < cols; j++) {
    //         cout << A[i][j] << " ";
    //     }
    //     cout << "\n" << flush;
    // }

    // CSR arrays
    int offset[MAX_ROWS] = {0};
    int col[MAX_NNZ];
    float val[MAX_NNZ];
    int size = 0;

    cout << std::endl;
    csr_format(A, rows, cols, offset, col, val, &size);

    cout << "\nCSR Format:"<<endl;

    ofstream fout(compressedFile + ".txt");

    if(fout.is_open()) 
    {
        fout << "Column indices"<<endl;
        for (int i = 0; i < size; i++) fout << col[i] << " ";


        fout << "\nNon-zero values"<<endl;
        for (int i = 0; i < size; i++) fout << val[i] << " ";
        fout << "\n" << flush;

        fout << "\nRow offsets"<<endl;
        for (int i = 0; i < rows; i++) fout << offset[i] << " ";
        fout << "\n" << flush;
        
        fout.close();
        cout << "Compression format generated" << endl; 
    } 
    
    else {
        cerr << "Error opening file for writing" << endl;
    }


    cout << "\nCSR Decompression:"<<endl;
    float A_decompress[MAX_ROWS][MAX_COLS] = {0};
    csr_decompression(A_decompress, rows, cols, offset, col, val, &size);
    ofstream fout_decompress(decompressedFile + ".txt");

    if(fout_decompress.is_open()) 
    {
        for(int i = 0; i < rows; i++) {
            for(int j = 0; j < cols; j++) {
                fout_decompress << A_decompress[i][j]<< " ";
            }
            fout_decompress << "\n";
        }
        fout_decompress.close();
        cout << "Decompressed matrix generated" << std::endl; 
    } 
    
    else {
        cerr << "Error opening file for writing" << std::endl;
    }

    cout << "\nFunctionality check:"<<endl;
    ifstream f1(matrixFile+ ".txt"), f2(decompressedFile + ".txt");
    if (!f1.is_open() || !f2.is_open()) {
        cerr << "Error opening files" << endl;
        return 1;
    }

    string line1, line2;
    int row = 0;
    bool identical = true;

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
