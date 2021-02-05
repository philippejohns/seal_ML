#include "verification.h"

//Implementations of (non-encrypted) logistic regression functions
vector<double> matrix_multiply(vector<vector<double>> & data, vector<double> & weights)
{
	vector<double> result;

	for (size_t i = 0; i < data.size(); i++)
	{
		double result_i = 0.0;
		for (size_t j = 0; j < data[i].size(); j++)
			result_i += data[i][j] * weights[j];
		
		result.push_back(result_i);
	}

	return result;
}

vector<double> sigmoid_function(vector<double> & result)
{	
	vector<double> sigmoid;

	for (size_t i = 0; i < result.size(); i++)
		sigmoid.push_back(1.0 / (1.0 + exp(-result[i])));

	return sigmoid;
}

//Used to determine accuracy of both encrypted and non-encrypted algorithms
double get_accuracy(vector<double> & sigmoid, vector<double> & y)
{
	const double YES = 1.0;
	const double NO = 0.0;
	size_t counter = 0;
	for (size_t i = 0; i < sigmoid.size(); i++)
		if ((sigmoid[i] >= THRESHOLD ? YES : NO) == y[i])
			counter++;

	return (double)((double) counter / (double) sigmoid.size());
}