#include <math.h>
#include <ctime>
#include <vector>
#include "read_files.h"
#include "seal_functions.h"

using namespace std;

vector<double> matrix_multiply(vector<vector<double>> & data, vector<double> & weights);
vector<double> sigmoid_function(vector<double> & result);
double get_accuracy(vector<double> & sigmoid, vector<double> & y);
