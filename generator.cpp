#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <bits/stdc++.h>

using namespace std;

const int rows = 1024;
const int cols = 1024;

vector<size_t> sparseMatrix(size_t N, size_t k, mt19937 &rng) {

    vector<size_t> positions;
    positions.reserve(k);

    // Trivial cases
    if (k == 0) return positions;
    if (k >= N) {
        // select all indices 0..N-1
        positions.resize(N);
        for (size_t i = 0; i < N; ++i) positions[i] = i;
        return positions;
    }

    std::uniform_real_distribution<double> u01(0.0, 1.0);
    size_t remainingToChoose = k;

    for (size_t idx = 0; idx < N && remainingToChoose > 0; ++idx) {
        size_t remainingItems = N - idx; 
        double pickProb = double(remainingToChoose) / double(remainingItems);
        if (u01(rng) < pickProb) {
            positions.push_back(idx);
            --remainingToChoose;
        }
    }

   
    if (positions.size() != k) {
        positions.clear();
        positions.reserve(k);
        for (size_t i = 0; i < k; ++i) positions.push_back(i);
    }

    return positions;
}

void saveMatrix(const vector<vector<int>>& mat, const string& filename) {
    ofstream fout(filename);
    int rows = mat.size();
    int cols = mat[0].size();

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fout << mat[i][j] << " ";
        }
        fout << "\n";
    }
}

// vector<vector<int>> readMatrix(const string& filename) {
//     ifstream fin(filename);
    
//     vector<vector<int>> mat(rows, vector<int>(cols));
//     for (int i = 0; i < rows; i++)
//         for (int j = 0; j < cols; j++)
//             fin >> mat[i][j];

//     return mat;
// }


int main() {

    int fileNum;
    cout << "Matrix file number: ";
    cin >> fileNum;
    string filename = "matrix" + to_string(fileNum) + ".txt";

    // initialize empty matrix with zeros
    vector<vector<int>> mat(rows, vector<int>(cols, 0));

    
    double densityValue;
    cout << "Matrix density (fraction 0.0-1.0 or absolute count): ";
    if (!(cin >> densityValue)) {
        cerr << "Invalid input for density\n";
        return 1;
    }

    size_t total = static_cast<size_t>(rows) * static_cast<size_t>(cols);
    
    size_t k;
    if (densityValue >= 0.0 && densityValue <= 1.0) {
        
        k = static_cast<size_t>(std::llround(densityValue * static_cast<double>(total)));
        
    } else if (densityValue > 1.0) {
        
        long long rounded = std::llround(densityValue);
        if (rounded < 0) rounded = 0;
        if (static_cast<unsigned long long>(rounded) > total) rounded = static_cast<long long>(total);
        k = static_cast<size_t>(rounded);
    } else {
        // negative values are invalid
        cerr << "Density must be non-negative\n";
        return 1;
    }

    // set up RNG
    std::random_device rd;
    std::mt19937 rng(rd());

    // get positions to fill (0..total-1)
    vector<size_t> positions = sparseMatrix(total, k, rng);

    // populate matrix: map linear index to (r, c)
    for (size_t p : positions) {
        size_t r = p / cols;
        size_t c = p % cols;
        mat[r][c] = 1;
    }

    saveMatrix(mat, filename);
    cout << "Sparse Matrix generated: " << filename << "\n";

    int count=0;
    for (const auto& row : mat) {
        for (const auto& val : row) 
            if(val==1) count++;
    }
    cout << "Expected non zero entries: " << (rows*cols*densityValue) << "\n";
    cout << "Non zero entries in matrix: " << count << "\n";

    // read back and print (keeps originasl behavior)
    // vector<vector<int>> readMat = readMatrix("matrix.txt");
    // for (const auto& row : readMat) {
    //     for (const auto& val : row) {
    //         cout << val << " ";
    //     }
    //     cout << "\n";
    // }

    return 0;
}