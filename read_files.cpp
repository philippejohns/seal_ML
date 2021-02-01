#include "read_files.h"

vector<double> read_1D_file(std::string filename, size_t n)
{
	vector<double> values;
	ifstream file(filename);
	string line;
	getline(file, line);
	stringstream iss1(line);
	double val;
	for (size_t i = 0; i < n; ++i)
	{
		string string_val;
		getline(iss1, string_val, ',');
		stringstream convertor(string_val);
		convertor >> val;
		values.push_back(val);
	}

	return values;
}

vector<vector<double>> read_2D_file(string filename, size_t rows, size_t cols)
{
	vector<vector<double>> values;
	ifstream file(filename);

	for (size_t row = 0; row < rows; ++row)
	{
		vector<double> row_values;
		string line;
		getline(file, line);
		if (!file.good())
			break;

		stringstream iss(line);

		for (size_t col = 0; col < cols; ++col)
		{
			string string_val;
			double val;
			getline(iss, string_val, ',');
			
			stringstream convertor(string_val);
			convertor >> val;
			row_values.push_back(val);
		}

		values.push_back(row_values);
	}
	return values;
}