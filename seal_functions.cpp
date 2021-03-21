#include "seal_functions.h"

Plaintext 
SEAL_encode_weights(CKKSEncoder & encoder, vector<double> & weights)
{
    vector<double> batched_weights;

    size_t rows_per_ciphertext = (NR_SLOTS / NR_COLS);

    for (size_t i = 0; i < rows_per_ciphertext; i++)
        batched_weights.insert(batched_weights.end(), weights.begin(), weights.end());

    Plaintext pt_weights;
    encoder.encode(batched_weights, CT_SCALE, pt_weights);

    return pt_weights;
}

vector<Ciphertext> 
SEAL_encrypt_matrix(Encryptor & encryptor, CKKSEncoder & encoder, vector<vector<double>> & matrix)
{
    size_t rows_per_ciphertext = (NR_SLOTS / NR_COLS);
    vector<vector<double>> batched_matrix;
    vector<Ciphertext> encrypted_matrix;

    size_t count = 0;

    for (auto row : matrix)
    {
        if ((count++ % rows_per_ciphertext) == 0)
            batched_matrix.emplace_back();

        batched_matrix.back().insert(batched_matrix.back().end(), row.begin(), row.end());
    }

    for (auto & batched_rows : batched_matrix)
    {
        Plaintext pt;
        encoder.encode(batched_rows, CT_SCALE, pt);
        Ciphertext ct;
        encryptor.encrypt(pt, ct);
        encrypted_matrix.push_back(ct);
    }

    return encrypted_matrix;
}

void
SEAL_matrix_multiply(Evaluator & evalr, GaloisKeys & gal_keys, vector<Ciphertext> & matrix, Plaintext & weights,
    vector<Ciphertext>::iterator start, vector<Ciphertext>::iterator end)
{
    for (auto it = start; it != end; it++)
        SEAL_dot_product(evalr, gal_keys, *it, weights);
}

void
SEAL_dot_product(Evaluator & evalr, GaloisKeys & gal_keys, Ciphertext & ct, Plaintext & pt)
{
    evalr.multiply_plain_inplace(ct, pt); 

    //sums up elements of each vector batched in ct*pt
    Ciphertext rotated;
    for (size_t i = NR_COLS / 2; i > 0; i /= 2)
    {
        evalr.rotate_vector(ct, i, gal_keys, rotated);
        evalr.add_inplace(ct, rotated);
    }
}

void
SEAL_sigmoid(Evaluator & evalr, CKKSEncoder & encoder, vector<Ciphertext> & vec, RelinKeys & r_keys, 
    vector<Ciphertext>::iterator start, vector<Ciphertext>::iterator end)
{
    Plaintext plain_coeff3, plain_coeff1, plain_scale_down;
    encoder.encode(COEFF1, CT_SCALE, plain_coeff1);
    encoder.encode(COEFF3, CT_SCALE, plain_coeff3);
    encoder.encode(COEFF_XSCALE, CT_SCALE, plain_scale_down);
    
    for (auto it = start; it != end; it++)
    { 
        Ciphertext x_scaled, x_scaled_coeff3, x_scaled_coeff1, x3;

        //x was not rescaled after a single multiplication in the matrix multiply
        //evalr.rescale_to_next_inplace(x);
        //evalr.mod_switch_to_inplace(plain_scale_down, x.parms_id());
        evalr.rescale_to_next_inplace(*it);
        evalr.mod_switch_to_inplace(plain_scale_down, (*it).parms_id());

        //scaling down x (the actual number, not the ciphertext)
        //evalr.multiply_plain(x, plain_scale_down, x_scaled);
        evalr.multiply_plain(*it, plain_scale_down, x_scaled);
        evalr.rescale_to_next_inplace(x_scaled);

        //to calculated x^3, square x first
        evalr.square(x_scaled, x3);
        evalr.relinearize_inplace(x3, r_keys);
        evalr.rescale_to_next_inplace(x3);

        //multiply x by coefficient before multiplying x with x^2
        evalr.mod_switch_to_inplace(plain_coeff3, x_scaled.parms_id());
        evalr.multiply_plain(x_scaled, plain_coeff3, x_scaled_coeff3);
        evalr.rescale_to_next_inplace(x_scaled_coeff3);
        
        //multiplying x^2 with coeff*x
        evalr.multiply_inplace(x3, x_scaled_coeff3);
        evalr.relinearize_inplace(x3, r_keys);

        //now calculating x_1
        evalr.mod_switch_to_inplace(plain_coeff1, x_scaled.parms_id());
        evalr.multiply_plain(x_scaled, plain_coeff1, x_scaled_coeff1);
        
        //modswitching so we can add them
        evalr.mod_switch_to_inplace(x_scaled_coeff1, x3.parms_id());
        
        //this is approximately the scale of both (determined experimentally), 
        //so set them to be exactly equal to allow adding them 
        x_scaled_coeff1.scale() = 1.20895e+24;
        x3.scale() = 1.20895e+24;

        evalr.add_inplace(x3, x_scaled_coeff1); //x3 now has the whole polynomial
        *it = x3;
    }

    //return result;
}

vector<double> 
SEAL_decrypt_result(Decryptor & decryptor, CKKSEncoder & encoder, vector<Ciphertext> & encrypted_result)
{
    vector<double> result;
    size_t nr_entries = NR_SLOTS / NR_COLS;

    for (auto & ct : encrypted_result)
    {   
        vector<double> batched_results;
        Plaintext pt;
        decryptor.decrypt(ct, pt);
        encoder.decode(pt, batched_results);
        
        if (&ct == &encrypted_result.back())                //this seems like it may be a bad idea 
            nr_entries = NR_ROWS % (NR_SLOTS / NR_COLS);    //maybe switch back to iterators

        for (size_t i = 0; i < nr_entries; i++)
            result.push_back(batched_results[i*NR_COLS]);
    }

    return result;
}