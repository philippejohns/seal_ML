#include "main.h"
#include <thread>

const size_t NR_ROWS = 422; 			//Number of rows in matrix of data
const size_t NR_COLS = 1024;			//Number of columns in matrix of data
const size_t NR_SLOTS = 8192;			//Number of slots in a ciphertext
const double CT_SCALE = pow(2.0, 40);	//Scale of ciphertext

const vector<int> PRIMES = { 60, 40, 40, 40, 40, 60 };

//Sigmoid approximation coefficients
//Taken from Kim et. al. paper
//sig_approx(x) = COEFF1(x*COEFF_XSCALE) + COEFF3(x*COEFF_XSCALE)^3
const double COEFF1 = 0.5 * 1.20096;
const double COEFF3 = -0.81562;
const double COEFF_XSCALE = 0.0125;		//The paper uses 1/8 to scale down x, however 
										//we have better results with 1/80 

const string X_filename = "../medical_data.csv";
const string weights_filename = "../medical_weights.csv";
const string y_filename = "../medical_correct_results.csv";

const double THRESHOLD = 0.5; 	//Sigmoid decision threshold

int main()
{	
	//Reading in data from files
	vector<vector<double>> data = read_2D_file(X_filename, NR_ROWS, NR_COLS);
	vector<double> weights = read_1D_file(weights_filename, NR_COLS);
	vector<double> y = read_1D_file(y_filename, NR_ROWS);
	
	//Performing algorithm in plaintext
	clock_t c_start = std::clock();		// start time for plaintext algorithm
	vector<double> correct_result = matrix_multiply(data, weights);
	vector<double> sigmoid = sigmoid_function(correct_result);
	clock_t c_end = clock(); 			//end time for plaintext algorithm

	//Calculating accuracy - This is where the comparison operator making the decision is
	double accuracy = get_accuracy(sigmoid, y);

	//Printing results
	double time_elapsed_ms = 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC;
	cout << "plaintext CPU time used: " << time_elapsed_ms << " ms" << endl;
	cout << "plaintext accuracy (exact sigmoid): "<< accuracy << endl;

	//Setting up encryption parameters, keys and objects
    EncryptionParameters parms(scheme_type::ckks);
    parms.set_poly_modulus_degree(NR_SLOTS * 2);	//poly_modulus_degree must be twice NR_SLOTS
    parms.set_coeff_modulus(CoeffModulus::Create(NR_SLOTS * 2, PRIMES));
    SEALContext context(parms);
    
    KeyGenerator keygen(context);
    auto secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    GaloisKeys gal_keys;
    keygen.create_galois_keys(gal_keys);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    
    Evaluator evaluator(context);
    CKKSEncoder encoder(context);
    Encryptor encryptor(context, public_key);
    Decryptor decryptor(context, secret_key);

	//Encoding weights in plaintext
    Plaintext plaintext_weights = SEAL_encode_weights(encoder, weights);

    //Encrypting matrix of data
    vector<Ciphertext> encrypted_matrix = SEAL_encrypt_matrix(encryptor, encoder, data);

    //Performing algorithm while encrypted with SEAL
    c_start = std::clock();	// start time
    //vector<Ciphertext> encrypted_result = SEAL_matrix_multiply(evaluator, gal_keys, encrypted_matrix, plaintext_weights);
    //vector<Ciphertext> sigmoid_result = SEAL_sigmoid(evaluator, encoder, encrypted_result, relin_keys);
    SEAL_matrix_multiply(evaluator, gal_keys, encrypted_matrix, plaintext_weights);
    SEAL_sigmoid(evaluator, encoder, encrypted_matrix, relin_keys);
    c_end = clock(); 		//end time

    //vector<double> sigmoid_decrypted = SEAL_decrypt_result(decryptor, encoder, sigmoid_result);
    vector<double> sigmoid_decrypted = SEAL_decrypt_result(decryptor, encoder, encrypted_matrix);

    //Calculating accuracy - This is where the comparison operator making the decision is
    accuracy = get_accuracy(sigmoid_decrypted, y);

    //Printing results
 	time_elapsed_ms = 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC;
	cout << "SEAL CPU time used: " << time_elapsed_ms << " ms" << endl;
	cout << "SEAL accuracy (3rd degree polynomial approx.): "<< accuracy << endl;

	return 0;
}
