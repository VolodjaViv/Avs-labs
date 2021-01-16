#include <iostream>
#include <omp.h>
#include <time.h>
#include <thread>
using namespace std;

void print_time(clock_t start, clock_t end, string type) {
    if(type == "ompOff") { cout << "Straight_result: " << (double)(end - start) / CLK_TCK << endl; }
    else if(type == "ompOn") { cout << "Dynamic parallel result: " << (double)(end - start) / CLK_TCK << endl; }
}

int main() {
    //initializing
    string type;
    const int n = 1000, m = 1000;
    clock_t start, end;
    int** fitstMatrix = new int* [n];
    int** secondMatrix = new int* [n];
    int** mullResMatrix = new int* [n];

    //filling matrix
    for (int i = 0; i < n; i++) {fitstMatrix[i] = new int[m]; secondMatrix[i] = new int[m]; mullResMatrix[i] = new int[m]; }
    for (int i = 0; i < n; i++) { for (int j = 0; j < m; j++) { fitstMatrix[i][j] = rand() % 20; secondMatrix[i][j] = rand() % 20; }}

    //------------------------------------------------------------------------------------------------------------------
    //straight counting without omp

    type = "ompOff";
    start = clock();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            mullResMatrix[i][j] = 0;
            for (int k = 0; k < n; k++)
                mullResMatrix[i][j] += fitstMatrix[i][k] * secondMatrix[k][j];
        }
    }
    end = clock();
    print_time(start, end, type);

    //------------------------------------------------------------------------------------------------------------------
    //counting using omp

    type = "ompOn";
    start = clock();
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                mullResMatrix[i][j] = 0;
                for (int k = 0; k < n; k++) mullResMatrix[i][j] += fitstMatrix[i][k] * secondMatrix[k][j];
            }
        }
    }
    end = clock();
    print_time(start, end, type);

    //------------------------------------------------------------------------------------------------------------------

    //free space
    for (int i = 0; i < n; i++) { delete[] fitstMatrix[i]; delete[] secondMatrix[i]; delete[] mullResMatrix[i]; }
    delete[] fitstMatrix;
    delete[] secondMatrix;
    delete[] mullResMatrix;
    return 0;
}