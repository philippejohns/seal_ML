#include "seal/seal.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace seal;
using namespace std;

extern const size_t NR_ROWS;
extern const size_t NR_COLS;
extern const size_t NR_SLOTS;
extern const double CT_SCALE;
extern const double COEFF1;
extern const double COEFF3;
extern const double COEFF_XSCALE;

Plaintext 
SEAL_encode_weights(CKKSEncoder & encoder, vector<double> & weights);

vector<Ciphertext> 
SEAL_encrypt_matrix(Encryptor & encryptor, CKKSEncoder & encoder, vector<vector<double>> & matrix);

//vector<Ciphertext>
void
SEAL_matrix_multiply(Evaluator & evalr, GaloisKeys & gal_keys, vector<Ciphertext> & matrix, Plaintext & weights,
    vector<Ciphertext>::iterator start, vector<Ciphertext>::iterator end);

//Ciphertext
void
SEAL_dot_product(Evaluator & evalr, GaloisKeys & gal_keys, Ciphertext & ct, Plaintext & pt);

//vector<Ciphertext> 
void
SEAL_sigmoid(Evaluator & evalr, CKKSEncoder & encoder, vector<Ciphertext> & vec, RelinKeys & r_keys,
	vector<Ciphertext>::iterator start, vector<Ciphertext>::iterator end);

vector<double> 
SEAL_decrypt_result(Decryptor & decryptor, CKKSEncoder & encoder, vector<Ciphertext> & encrypted_result);