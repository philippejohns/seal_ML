#include <math.h>
#include <vector>

using namespace std;

extern const double THRESHOLD;

vector<double> matrix_multiply(vector<vector<double>> & data, vector<double> & weights);
vector<double> sigmoid_function(vector<double> & result);
double get_accuracy(vector<double> & sigmoid, vector<double> & y);