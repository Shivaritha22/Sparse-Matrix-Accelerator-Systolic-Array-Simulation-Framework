#include <iostream>
#include <fstream>
#include <filesystem>
#include <cmath>
using namespace std;

#include "bcsr.h"

int main() {
    const int GLOBAL = 1024;
    const int TILE = MAX_ROWS;
    const int TILES = GLOBAL / TILE;
    const int BRows = BR;
    const int VPER = values_per_block;

    int fileNum;
    cout << "Enter matrix file number: ";
    cin >> fileNum;

    string matrixFile = "matrix" + to_string(fileNum);
    string compressedFile = "compressed" + to_string(fileNum);
    string decompressedFile = "decompressed" + to_string(fileNum);
    string resultFile = "result" + to_string(fileNum);

    const string outDir = "data" + to_string(fileNum);
    const string rawDir = outDir + "/raw";
    const string compDir = outDir + "/compressions";
    const string decompDir = outDir + "/decompressions";
    std::filesystem::create_directories(rawDir);
    std::filesystem::create_directories(compDir);
    std::filesystem::create_directories(decompDir);

    static float A_big[GLOBAL][GLOBAL];
    ifstream fin(matrixFile + ".txt");
    if (!fin.is_open()) {
        cerr << "Error opening " << matrixFile << ".txt" << endl;
        return 1;
    }
    for (int i = 0; i < GLOBAL; ++i)
        for (int j = 0; j < GLOBAL; ++j)
            fin >> A_big[i][j];
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
    for (int i = 0; i < GLOBAL; ++i)
        for (int j = 0; j < GLOBAL; ++j)
            A_big_decomp[i][j] = 0.0f;

    long long total_meta_elems = 0;
    long long total_data_elems = 0;
    long long tile_count = 0;
    long long total_cycles_all_tiles = 0;

    float A_tile[TILE][TILE];
    float A_tile_decomp[TILE][TILE];

    int   row_ptr[MAX_ROWS + 1] = {0};
    int   col[MAX_NNZ] = {0};
    float val[MAX_NNZ] = {0.0f};
    int   size_blocks = 0;

    for (int tr = 0; tr < TILES; ++tr) {
        for (int tc = 0; tc < TILES; ++tc) {
            int tile_index = tr * TILES + tc + 1;

            // slice tile
            for (int i = 0; i < TILE; ++i)
                for (int j = 0; j < TILE; ++j)
                    A_tile[i][j] = A_big[tr*TILE + i][tc*TILE + j];

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

            // clear outputs
            for (int i = 0; i < MAX_ROWS + 1; ++i) row_ptr[i] = 0;
            for (int i = 0; i < MAX_NNZ; ++i)      col[i] = 0;
            for (int i = 0; i < MAX_NNZ; ++i)      val[i] = 0.0f;
            size_blocks = 0;

            bcsr_format(A_tile, TILE, TILE, row_ptr, col, val, &size_blocks);

            total_meta_elems += size_blocks + BRows;
            total_data_elems += (long long)size_blocks * VPER;
            tile_count       += 1;

            fout << "=== Tile (" << tr << "," << tc << ") ===\n";
            fout << "blocks\n" << size_blocks << "\n";
            fout << "Column indices\n";
            for (int i = 0; i < size_blocks; ++i) fout << col[i] << " ";
            fout << "\n";
            fout << "Non-zero block values\n";
            for (int i = 0; i < size_blocks * VPER; ++i) fout << val[i] << " ";
            fout << "\n";
            fout << "Row offsets\n";
            for (int i = 1; i <= BRows; ++i) fout << row_ptr[i] << " ";
            fout << "\n\n";

            {
                ofstream fcomp(compDir + "/tile_comp" + to_string(tile_index) + ".txt");
                if (!fcomp.is_open()) {
                    cerr << "Error opening " << compDir << "/tile_comp" << tile_index << ".txt" << endl;
                    return 1;
                }
                fcomp << "blocks\n" << size_blocks << "\n";
                fcomp << "Column indices\n";
                for (int i = 0; i < size_blocks; ++i) fcomp << col[i] << " ";
                fcomp << "\n";
                fcomp << "Non-zero block values\n";
                for (int i = 0; i < size_blocks * VPER; ++i) fcomp << val[i] << " ";
                fcomp << "\n";
                fcomp << "Row offsets\n";
                for (int i = 1; i <= BRows; ++i) fcomp << row_ptr[i] << " ";
                fcomp << "\n";
                fcomp.close();
            }

            // clear decomp tile
            for (int i = 0; i < TILE; ++i)
                for (int j = 0; j < TILE; ++j)
                    A_tile_decomp[i][j] = 0.0f;

            decompress_bcsr(row_ptr + 1, col, val, size_blocks, A_tile_decomp);

            int cycles_tile = size_blocks + 2*BRows - 1;
            total_cycles_all_tiles += cycles_tile;
            flog << "Tile " << tile_index << " (" << tr << "," << tc << "): "
                 << "cycles " << cycles_tile << " (blocks=" << size_blocks << ")\n";

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

            // functional check
            {
                ifstream t1(rawDir + "/tile" + to_string(tile_index) + ".txt");
                ifstream t2(decompDir + "/tile_decomp" + to_string(tile_index) + ".txt");
                if (!t1.is_open() || !t2.is_open()) {
                    cerr << "Error opening tile comparison files for tile " << tile_index << endl;
                    return 1;
                }
                for (int i = 0; i < TILE; ++i) {
                    for (int j = 0; j < TILE; ++j) {
                        float a, b;
                        if (!(t1 >> a) || !(t2 >> b)) {
                            cout << "Tile (" << tr << "," << tc << ") early EOF while comparing at row "
                                 << (i+1) << ", col " << (j+1) << endl;
                            break;
                        }
                        if (fabs(a - b) > 1e-6f) {
                            cout << "Tile (" << tr << "," << tc << ") difference at row "
                                 << (i+1) << ", col " << (j+1) << " : " << a << " vs " << b << endl;
                        }
                    }
                }
            }

            for (int i = 0; i < TILE; ++i)
                for (int j = 0; j < TILE; ++j)
                    A_big_decomp[tr*TILE + i][tc*TILE + j] = A_tile_decomp[i][j];
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

    // functional check
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
            cout << "BCSR Compression and Decompression: Successful" << endl;
        }
    }

    double meta_data_ratio = (total_data_elems == 0)
        ? 0.0
        : static_cast<double>(total_meta_elems) / static_cast<double>(total_data_elems);

    double avg_cycles_per_tile = (tile_count == 0)
        ? 0.0
        : static_cast<double>(total_cycles_all_tiles) / static_cast<double>(tile_count);

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
        fsum << "\nTotal BCSR decompression cycles (all tiles): " << total_cycles_all_tiles;
        fsum << "\nAverage BCSR decompression cycles per tile: " << avg_cycles_per_tile << endl;

        fsum.close();
    }

    return 0;
}
