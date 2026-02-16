#include <iostream>
#include <fstream>
#include "coo.h"
using namespace std;

int main() {
    
    float A[MAX_ROWS][MAX_COLS] = {0};
    int rows = MAX_ROWS, cols = MAX_COLS;
    int size=0;

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

    
    int tuple_A[MAX_NNZ][3];

    coo_format(A, tuple_A, &size);

    cout << "\nCOO Format:"<<endl;
    ofstream fout(compressedFile + ".txt");

    fout << "size: "<<size<<endl;
    if(fout.is_open()) 
    {
        fout << "Tuples"<<endl;
        for (int i = 0; i < size; i++) {
            for(int j=0; j<3; j++)
                fout << tuple_A[i][j] << " " << (j == 1 ? " " : "");
            fout << endl;
        }

        fout.close();
        cout << "Compression format generated" << endl; 
    } 
    
    else {
        cerr << "Error opening file for writing" << endl;
    }


    cout << "\nCOO Decompression:"<<endl;
    float A_decompress[MAX_ROWS][MAX_COLS] = {0};
    coo_decompression(A_decompress, tuple_A, &size);

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

    // functionality check
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
