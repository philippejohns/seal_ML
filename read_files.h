#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

vector<double> read_1D_file(string filename, size_t n);
vector<vector<double>> read_2D_file(string filename, size_t rows, size_t cols);