#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <omp.h>
#include <fstream>
#include <algorithm>

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (a) > (b) ? (b) : (a)

bool checkThreads(char *string);

using namespace std;


int main(int argc, char *argv[]) {
    ofstream output;
    ifstream input;
    int threads;
    double coefficient;
    try {
        if (argc != 5) {
            throw invalid_argument("Expected 5 arguments, got " + to_string(argc));
        }
        if (!checkThreads(argv[1])) {
            throw invalid_argument("Invalid threads");
        }
        threads = stoi(argv[1]);
        input.open(argv[2], ios::binary);
        output.open(argv[3], ios::binary);
        coefficient = stod(argv[4]);
        if (!(coefficient >= 0.0 && coefficient < 0.5)) {
            throw invalid_argument("coefficient doesn't satisfy the requirements");
        }
        if (!input.is_open() || !output.is_open()) {
            throw invalid_argument("Couldn't locate the files necessary");
        }
        string header;
        long width, height, Maxval;
        input >> header;
        input.get();
        char t = input.get();
        while (t != '\n') {
            t = input.get();
        }
        input >> width >> height >> Maxval;
        if (Maxval != 255) {
            throw invalid_argument("Maxval expected 255, got " + to_string(Maxval));
        }
        input.get();

        if (endsWith(argv[2], ".ppm") || (endsWith(argv[2], ".pnm") && header == "P6")) {
            if (endsWith(argv[3], ".pgm")) {
                throw invalid_argument("Mismatch in formats (input and output are of different color type)");
            }
            if (header != "P6") {
                throw invalid_argument("Input is .ppm but not P6");
            }

            long size = width * height;
            vector<unsigned char> px(size * 3);
            input.read((char *) &px[0], size * 3);
            input.close();
            double startTime = omp_get_wtime();
            int colors[256][3] = {{}};
            if (threads != 0) {
                omp_set_num_threads(threads);
            }
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    threads = omp_get_num_threads();
                }
                int j, k, i;
                const int nSize = size;
                int tmpcolors[256][3] = {{}};
#pragma omp for schedule(static)
                for (i = 0; i < nSize; i++) {
                    tmpcolors[px[i]][i % 3]++;
                    tmpcolors[px[nSize + i]][(nSize + i) % 3]++;
                    tmpcolors[px[2 * nSize + i]][(2 * nSize + i) % 3]++;
                }
#pragma omp critical
                {
                    for (j = 0; j < 256; j++) {
                        for (k = 0; k < 3; k++) {
                            colors[j][k] += tmpcolors[j][k];
                        }
                    }
                }
            }
            long amount = (long) (size * coefficient);
            long leftSums[3] = {};
            long rightSums[3] = {};
            int leftBound = -1;
            for (int i = 0; i < 256; i++) {
                for (int j = 0; j < 3; j++) {
                    leftSums[j] += colors[i][j];
                    if (leftSums[j] > amount) {
                        // was not initialized
                        leftBound = i;
                        break;
                    }
                }
                if (leftBound != -1)break;
            }
            int rightBound = -1;
            for (int i = 255; i >= 0; i--) {
                for (int j = 0; j < 3; j++) {
                    rightSums[j] += colors[i][j];
                    if (rightSums[j] > amount) {
                        // was not initialized
                        rightBound = i;
                        break;
                    }
                }
                if (rightBound != -1)break;
            }
            if (rightBound != leftBound) {
#pragma omp parallel
                {
                    float formula = 255 / float(rightBound - leftBound), tmp, lb = leftBound;
#pragma omp for
                    for (int i = 0; i < size * 3; i++) {
                        tmp = formula * float(px[i] - lb);
                        px[i] = (min(255, max(0, tmp)));
                    }
                }
            }
            double endTime = omp_get_wtime();
            printf("Time (%i thread(s)): %g ms\n", threads, endTime - startTime);
            output << "P6\n" << width << " " << height << "\n255\n";
            output.write((char *) px.data(), size * 3);
            output.close();
        } else if (endsWith(argv[2], ".pgm") || (endsWith(argv[2], ".pnm") && header == "P5")) {
            if (endsWith(argv[3], ".ppm")) {
                throw invalid_argument("Mismatch in formats (input and output are of different color type)");
            }
            if (header != "P5") {
                throw invalid_argument("Input is .pgm but not P5");
            }
            long size = width * height;
            vector<unsigned char> px(size);
            input.read((char *) &px[0], size);
            input.close();
            double startTime = omp_get_wtime();
            int colors[256] = {0};
            if (threads != 0) {
                omp_set_num_threads(threads);
            }
#pragma omp parallel
            {
                if (omp_get_thread_num() == 0) {
                    threads = omp_get_num_threads();
                }
                int j, i;
                const int nSize = size;
                vector<unsigned char> &addr = px;
                int tmpcolors[256] = {};
#pragma omp for schedule(static, 1)
                for (i = 0; i < nSize; i++) {
                    tmpcolors[addr[i]]++;
                }
#pragma omp critical
                {
                    for (j = 0; j < 256; j++) {
                        colors[j] += tmpcolors[j];
                    }
                }
            }
            long amount = (long) (size * coefficient);

            long leftSums = 0;
            long rightSums = 0;
            int leftBound = -1;
            for (int i = 0; i < 256; i++) {
                leftSums += colors[i];
                if (leftSums > amount) {
                    leftBound = i;
                    break;
                }
            }
            int rightBound = -1;
            for (int i = 255; i >= 0; i--) {
                rightSums += colors[i];
                if (rightSums > amount) {
                    // was not initialized
                    rightBound = i;
                    break;
                }
            }
            if (rightBound != leftBound) {
                if (threads != 0) {
                    omp_set_num_threads(threads);
                }
#pragma omp parallel
                {
                    vector<unsigned char> &addr = px;
                    float formula = 255 / float(rightBound - leftBound), tmp, lb = leftBound;
#pragma omp for schedule(static, 1)
                    for (int i = 0; i < size; i++) {
                        tmp = formula * float(addr[i] - lb);
                        px[i] = (min(255, max(0, tmp)));
                    }
                }
            }
            double endTime = omp_get_wtime();
            printf("Time (%i thread(s)): %g ms\n", threads, endTime - startTime);
            output << "P5\n" << width << " " << height << "\n255\n";
            output.write((char *) px.data(), size);
            output.close();
        } else {
            throw invalid_argument(" Input is not of supported type");
        }
    } catch (const std::exception &e) {
        cout << e.what();
    } catch (...) {
        cout << "Was not supposed to happen";
    }
}

bool checkThreads(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        if (!(string[i] >= '0' && string[i] <= '9')) {
            return false;
        }

    }
    return true;
}
