#include "lil.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    int rows = SIZE, cols = SIZE;

    int fileNum;
    cout << "Enter matrix file number: ";
    cin >> fileNum;

    string matrixFile       = "matrix" + to_string(fileNum);
    string compressedFile   = "compressed" + to_string(fileNum);
    string decompressedFile = "decompressed" + to_string(fileNum);

    vector<vector<int>> A(rows, vector<int>(cols, 0));

    ifstream fin(matrixFile + ".txt");
    if (!fin.is_open()) {
        cerr << "Error: could not open " << matrixFile << ".txt\n";
        return 1;
    }
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            fin >> A[i][j];
    fin.close();
    cout << "\nLIL Format:"<<endl;
    vector<vector<int>> row_c, val_c;
    lil_format(A, row_c, val_c);

    
    ofstream ofs(compressedFile + ".txt");
    if (!ofs.is_open()) return 1;

    
    int max_depth = 0;
    for (int j = 0; j < SIZE; ++j)
        if ((int)row_c[j].size() > max_depth) max_depth = (int)row_c[j].size();

    ofs << "row\n";
    for (int k = 0; k < max_depth; ++k) {
        for (int j = 0; j < SIZE; ++j) {
            int r = (k < (int)row_c[j].size()) ? row_c[j][k] : -1;
            ofs << r << (j + 1 < SIZE ? ' ' : '\n');
        }
    }

    ofs << "values\n";
    for (int k = 0; k < max_depth; ++k) {
        for (int j = 0; j < SIZE; ++j) {
            int v = (k < (int)val_c[j].size()) ? val_c[j][k] : 0;
            ofs << v << (j + 1 < SIZE ? ' ' : '\n');
        }
    }
    ofs.close();
    cout << "Compression format generated" << endl; 
    
    cout << "\nLIL Decompression:"<<endl;
    vector<vector<int>> A_dec, BRAM;
    int cycles = lil_decompress(row_c, val_c, A_dec, BRAM);
    cout << "Total decompression cycles: "<<cycles<<endl;

    
    ofstream ofd(decompressedFile + ".txt");
    if (!ofd.is_open()) return 1;

    for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
        ofd << A_dec[i][j] << " ";
    }
    ofd << "\n";
    }
    ofd.close();
    cout << "Decompressed matrix generated" << std::endl; 

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
