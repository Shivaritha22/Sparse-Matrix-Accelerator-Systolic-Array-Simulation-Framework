#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
#include "coo.h"
using namespace std;

int main() {
    // --- global (big) matrix size and tiling ---
    const int GLOBAL = 1024;            // full matrix is 1024x1024
    const int TILE   = MAX_ROWS;        // tile side is 64 (from coo.h)
    const int TILES  = GLOBAL / TILE;   // 16 tiles per dimension

    int fileNum;
    cout << "Enter matrix file number: ";
    cin >> fileNum;

    string matrixFile       = "matrix" + to_string(fileNum);
    string compressedFile   = "compressed" + to_string(fileNum);
    string decompressedFile = "decompressed" + to_string(fileNum);
    string resultFile       = "result" + to_string(fileNum);

    // folder structure
    const string outDir    = "data" + to_string(fileNum);
    const string rawDir    = outDir + "/raw";
    const string compDir   = outDir + "/compressions";
    const string decompDir = outDir + "/decompressions";
    std::filesystem::create_directories(rawDir);
    std::filesystem::create_directories(compDir);
    std::filesystem::create_directories(decompDir);

    // read
    static float A_big[GLOBAL][GLOBAL];
    ifstream fin(matrixFile + ".txt");
    if (!fin.is_open()) {
        cerr << "Error opening " << matrixFile << ".txt" << endl;
        return 1;
    }
    for (int i = 0; i < GLOBAL; ++i) {
        for (int j = 0; j < GLOBAL; ++j) {
            fin >> A_big[i][j];
        }
    }
    fin.close();

    
    ofstream fout(compDir + "/" + compressedFile + ".txt");
    if (!fout.is_open()) {
        cerr << "Error opening " << compDir << "/" << compressedFile << ".txt" << endl;
        return 1;
    }

    
    ofstream flog(decompDir + "/log_decompression_cycle.txt");
    if (!flog.is_open()) {
        cerr << "Error opening " << decompDir << "/log_decompression_cycle.txt" << endl;
        return 1;
    }

    
    static float A_big_decomp[GLOBAL][GLOBAL];
    for (int i = 0; i < GLOBAL; ++i) {
        for (int j = 0; j < GLOBAL; ++j) {
            A_big_decomp[i][j] = 0.0f;
        }
    }

    
    long long total_meta_elems = 0;
    long long total_data_elems = 0;
    long long tile_count = 0;
    long long total_cycles_all_tiles = 0;

    
    float A_tile[TILE][TILE];
    float A_tile_decomp[TILE][TILE];
    int   tuple_A[MAX_NNZ][3];
    int   size = 0;

    
    for (int tr = 0; tr < TILES; ++tr) {
        for (int tc = 0; tc < TILES; ++tc) {
            int tile_index = tr * TILES + tc + 1;

            // slice tile from big matrix into A_tile
            for (int i = 0; i < TILE; ++i) {
                for (int j = 0; j < TILE; ++j) {
                    A_tile[i][j] = A_big[tr*TILE + i][tc*TILE + j];
                }
            }

            
            {
                ofstream ftile(rawDir + "/tile" + to_string(tile_index) + ".txt");
                if (!ftile.is_open()) {
                    cerr << "Error opening " << rawDir << "/tile" << tile_index << ".txt" << endl;
                    return 1;
                }
                for (int i = 0; i < TILE; ++i) {
                    for (int j = 0; j < TILE; ++j) {
                        ftile << A_tile[i][j];
                        if (j + 1 < TILE) ftile << ' ';
                    }
                    ftile << "\n";
                }
                ftile.close();
            }

            // clear COO outputs for this tile
            for (int i = 0; i < MAX_NNZ; ++i) {
                tuple_A[i][0] = 0;
                tuple_A[i][1] = 0;
                tuple_A[i][2] = 0;
            }
            size = 0;

            coo_format(A_tile, tuple_A, &size);

            total_meta_elems = total_meta_elems + (2 * size);
            total_data_elems = total_data_elems + size;
            tile_count       = tile_count + 1;

            // append this tile's COO to the single compressed file
            fout << "=== Tile (" << tr << "," << tc << ") ===\n";
            fout << "size: " << size << "\n";
            fout << "Tuples\n";
            for (int i = 0; i < size; ++i) {
                fout << tuple_A[i][0] << " " << tuple_A[i][1] << "  " << tuple_A[i][2] << "\n";
            }
            fout << "\n";

            {
                ofstream fcomp(compDir + "/tile_comp" + to_string(tile_index) + ".txt");
                if (!fcomp.is_open()) {
                    cerr << "Error opening " << compDir << "/tile_comp" << tile_index << ".txt" << endl;
                    return 1;
                }
                fcomp << "size: " << size << "\n";
                fcomp << "Tuples\n";
                for (int i = 0; i < size; ++i) {
                    fcomp << tuple_A[i][0] << " " << tuple_A[i][1] << "  " << tuple_A[i][2] << "\n";
                }
                fcomp.close();
            }

            for (int i = 0; i < TILE; ++i) {
                for (int j = 0; j < TILE; ++j) {
                    A_tile_decomp[i][j] = 0.0f;
                }
            }
            int size_for_decomp = size;
            coo_decompression(A_tile_decomp, tuple_A, &size_for_decomp);

            int cycles_tile = size;
            total_cycles_all_tiles = total_cycles_all_tiles + cycles_tile;

            flog << "Tile " << tile_index << " (" << tr << "," << tc << "): "
                 << "cycles " << cycles_tile << " (nnz=" << size << ")\n";

            {
                ofstream fdec(decompDir + "/tile_decomp" + to_string(tile_index) + ".txt");
                if (!fdec.is_open()) {
                    cerr << "Error opening " << decompDir << "/tile_decomp" << tile_index << ".txt" << endl;
                    return 1;
                }
                for (int i = 0; i < TILE; ++i) {
                    for (int j = 0; j < TILE; ++j) {
                        fdec << A_tile_decomp[i][j];
                        if (j + 1 < TILE) fdec << ' ';
                    }
                    fdec << "\n";
                }
                fdec.close();
            }

            // functionality check per tile
            {
                ifstream t1(rawDir + "/tile" + to_string(tile_index) + ".txt");
                ifstream t2(decompDir + "/tile_decomp" + to_string(tile_index) + ".txt");
                if (!t1.is_open() || !t2.is_open()) {
                    cerr << "Error opening tile comparison files for tile " << tile_index << endl;
                    return 1;
                }
                bool identical = true;
                for (int i = 0; i < TILE; ++i) {
                    for (int j = 0; j < TILE; ++j) {
                        float a, b;
                        if (!(t1 >> a) || !(t2 >> b)) {
                            cout << "Tile (" << tr << "," << tc << ") early EOF while comparing at row "
                                 << (i+1) << ", col " << (j+1) << endl;
                            identical = false;
                            break;
                        }
                        if (fabs(a - b) > 1e-6f) {
                            cout << "Tile (" << tr << "," << tc << ") difference at row "
                                 << (i+1) << ", col " << (j+1) << " : " << a << " vs " << b << endl;
                            identical = false;
                        }
                    }
                }
                (void)identical;
            }

            for (int i = 0; i < TILE; ++i) {
                for (int j = 0; j < TILE; ++j) {
                    A_big_decomp[tr*TILE + i][tc*TILE + j] = A_tile_decomp[i][j];
                }
            }
        }
    }
    fout.close();
    flog.close();

    {
        ofstream fout_decompressed(decompDir + "/" + decompressedFile + ".txt");
        if (!fout_decompressed.is_open()) {
            cerr << "Error opening " << decompDir << "/" << decompressedFile << ".txt" << endl;
            return 1;
        }
        for (int i = 0; i < GLOBAL; ++i) {
            for (int j = 0; j < GLOBAL; ++j) {
                fout_decompressed << A_big_decomp[i][j];
                if (j + 1 < GLOBAL) fout_decompressed << ' ';
            }
            fout_decompressed << "\n";
        }
        fout_decompressed.close();
    }

    // functionality check
    {
        ifstream f1(matrixFile + ".txt"),
                 f2(decompDir + "/" + decompressedFile + ".txt");
        if (!f1.is_open() || !f2.is_open()) {
            cerr << "Error opening files for comparison" << endl;
            return 1;
        }

        bool identical = true;
        for (int i = 0; i < GLOBAL; ++i) {
            for (int j = 0; j < GLOBAL; ++j) {
                float a, b;
                if (!(f1 >> a) || !(f2 >> b)) {
                    cout << "Difference found in full matrix due to early EOF at row "
                         << (i+1) << ", col " << (j+1) << endl;
                    identical = false;
                    break;
                }
                if (fabs(a - b) > 1e-6f) {
                    cout << "Difference found in full matrix at row "
                         << (i+1) << ", col " << (j+1) << " : " << a << " vs " << b << endl;
                    identical = false;
                }
            }
        }
        if (identical) {
            cout << "COO Compression and Decompression: Successful" << endl;
        }
    }

    // output
    double meta_data_ratio;
    if (total_data_elems == 0) {
        meta_data_ratio = 0.0;
    } else {
        meta_data_ratio = static_cast<double>(total_meta_elems) /
                          static_cast<double>(total_data_elems);
    }

    double avg_cycles_per_tile;
    if (tile_count == 0) {
        avg_cycles_per_tile = 0.0;
    } else {
        avg_cycles_per_tile = static_cast<double>(total_cycles_all_tiles) /
                              static_cast<double>(tile_count);
    }

    {
        ofstream fsum(resultFile + ".txt");
        if (!fsum.is_open()) {
            cerr << "Error opening " << resultFile << ".txt" << endl;
            return 1;
        }
        fsum << "Storage Overhead:";
        fsum << "\nOverall metadata/data ratio (elements): " << meta_data_ratio;

        fsum << "\nDecompression Latency:";
        fsum << "\nTiles processed: " << tile_count;
        fsum << "\nTotal COO decompression cycles (all tiles): " << total_cycles_all_tiles;
        fsum << "\nAverage COO decompression cycles per tile: " << avg_cycles_per_tile << endl;

        fsum.close();
    }

    return 0;
}
