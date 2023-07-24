#include <cmath>
#include <cstring>

#include "logistic_regression.h"
#include "gwas.h"

/////////////////////////////////////////////////////////////
//////////              Log_row             /////////////////
/////////////////////////////////////////////////////////////

// Approximates e^-x from (-3, 3), and uses a step function after that. Good balance of accuracy and speed for our sigmoid function!
// https://math.stackexchange.com/questions/71357/approximation-of-e-x
inline double modified_pade_approx_oblivious(double x) {
    double approx = ((x + 3) * (x + 3) + 3) / ((x - 3) * (x - 3) + 3);
    int within_bounds = (x > -3) & (x < 3);
    int pos = x > 0;
    return approx * within_bounds + !within_bounds * ((pos << 7) * x);
}

// https://math.stackexchange.com/questions/71357/approximation-of-e-x
inline double modified_pade_approx(double x) {
    if (x < -3 || x > 3) {
        return x > 0 ? 100 * x : 0;
    }
    return ((x + 3) * (x + 3) + 3) / ((x - 3) * (x - 3) + 3);
}

Log_row::Log_row(int _size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy, int thread_id) : 
    Row(_size, sizes, _gwas->dim(), _impute_policy), H(num_dimensions, 2) {
    fitted = true;

    offset = thread_id * get_padded_buffer_len(num_dimensions);

    //beta_delta.resize(num_dimensions);
    // 2 is a magic number that helps with SqrMatrix construction, "highest level matrix"
    //Grad.resize(num_dimensions);
    //Grad = new double[num_dimensions];

    if (gwas->size() != n) throw CombineERROR("row length mismatch");
    // if (b.size() != n) {
    //     b.resize(n);
    // }
}

/* fitting */
bool Log_row::fit(int thread_id, int max_it, double sig) {
    /* intialize beta to 0*/

    init();
    it_count = 1;

    while (it_count < max_it && bd_max(beta_delta_g + offset, num_dimensions) >= sig) {
        update_beta();
        it_count++;
    }

    if (it_count == max_it) {
        fitted = false;
        return false;
    }
    else {
        H.INV();
        standard_error = std::sqrt(H.t[0][0]);
        return true;
    }
}

/* output results*/
double Log_row::get_beta(int thread_id) {
    if (!fitted) {
        return nan("");
    }
    return beta_g[0];//.front();
}

double Log_row::get_t_stat(int thread_id) {
    if (!fitted) {
        return nan("");
    }
    return beta_g[0] / standard_error;
}

double Log_row::get_standard_error(int thread_id) {
    if (!fitted) {
        return nan("");
        fitted = true;
    }
    return standard_error;
}

void Log_row::get_outputs(int thread_id, std::string& output_string) {
    if (!fitted) {
        output_string += "\tNA\tNA\tNA";
        fitted = true;
        return;
    }
    output_string += "\t" + std::to_string((beta_g + offset)[0]) +
                     "\t" + std::to_string(standard_error) +
                     "\t" + std::to_string((beta_g + offset)[0] / standard_error);

}

/* fitting helper functions */

void Log_row::update_beta() {
    // calculate_beta
    H.INV();
    H.calculate_t_matrix_times_vec(Grad_g + offset, beta_delta_g + offset);
    for (int i = 0; i < num_dimensions; i++) {
        double bd_i = (beta_delta_g + offset)[i];
        (beta_g + offset)[i] += bd_i;
        // take abs after adding beta delta so that we can determine if we have passed tolerance
        (beta_delta_g + offset)[i] = std::abs(bd_i);
    }

    update_estimate();
}

void Log_row::init() {
    for (int i = 0; i < num_dimensions; ++i) {
        (beta_delta_g + offset)[i] = 1;
        (beta_g + offset)[i] = 0;
    }

    //if (impute_average) {
        double sum = 0;
        double count = 0;
        uint8_t val;
        for (int i = 0 ; i < n; ++i) {
            val = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
            if (is_NA_uint8(val)) {
                sum += val;
                count++;
            }
        }

        genotype_average = sum / (count + !count);
    //}

    update_estimate();
}

void Log_row::update_estimate() {
    for (int i = 0; i < num_dimensions; ++i) {
        (Grad_g + offset)[i] = 0;
        for (int j = 0; j < num_dimensions; ++j){
            H.assign(i, j, 0);
        }
    }
    double y_est;
    unsigned int data_idx = 0, client_offset = 0, client_idx = 0;
    bool is_NA;
    for (int i = 0; i < n; i++) {
        double x = (data[(i + client_offset) / 4] >> (((i + client_offset) % 4) * 2) ) & 0b11;
        // is_NA = is_NA_uint8(x);
        // x = impute_average && is_NA ? genotype_average : x;
        // if (!is_NA && !impute_average) {
            const std::vector<double>& patient_pnc = gwas->phenotype_and_covars.data[i];

            y_est = (beta_g + offset)[0] * x;
            for (int j = 1; j < num_dimensions; j++) {
                y_est += patient_pnc[j] * (beta_g + offset)[j];
            }
            y_est = 1 / (1 + modified_pade_approx_oblivious(-y_est));

            update_upperH_and_Grad(y_est, x, patient_pnc);
        //}
        /* update data index */
        data_idx++;
        // int data_idx_lt_lengths = data_idx < client_lengths[client_idx];
        // // if data_idx >= lengths, data_idx = 0 - otherwise multiply by 1
        // data_idx *= data_idx_lt_lengths;
        // // if data_idx >= lengths, increment client_offset, otherwise multiply by 0
        // client_offset += (~data_idx_lt_lengths + 2) * ((4 - ((1 + i + client_offset) % 4)) % 4);
        if (data_idx >= client_lengths[client_idx]) {
            data_idx = 0;
            client_idx++;
            client_offset += (4 - ((1 + i + client_offset) % 4)) % 4; // how many pairs of bits do we need to skip?
        }
    }
    /* build lower half of H */
    for (int j = 0; j < num_dimensions; j++) {
        for (int k = j + 1; k < num_dimensions; k++) {
            H.inner_assign(j, k, k, j);
        }
    }
}

//Good!
void Log_row::update_upperH_and_Grad(double y_est, double x, const std::vector<double>& patient_pnc) {
    double y_est_1_y = y_est * (1 - y_est);
    double y_delta = patient_pnc[0] - y_est;
    (Grad_g + offset)[0] += y_delta * x;
    H.plus_equals(0, 0, x * x * y_est_1_y);
    for (int j = 1; j < num_dimensions; j++) {
        double patient_pnc_j = patient_pnc[j];
        double pnc_j_times_y_est = patient_pnc_j * y_est_1_y;
        (Grad_g + offset)[j] += y_delta * patient_pnc_j;
        H.plus_equals(j, 0, x * pnc_j_times_y_est);
        H.plus_equals(j, j, patient_pnc_j * pnc_j_times_y_est);

        for (int k = 1; k < j; k += 1) {
            H.plus_equals(j, k, patient_pnc[k] * pnc_j_times_y_est);
        }
    }
}

// //Good!
// void Log_row::update_upperH_and_Grad(double y_est, double x, const std::vector<double>& patient_pnc) {
//     double y_est_1_y = y_est * (1 - y_est);
//     double y_delta = patient_pnc[0] - y_est;
//     Grad[0] += y_delta * x;
//     H[0][0] += x * x * y_est_1_y;

//     for (int j = 1; j < num_dimensions; j++) {
//         double patient_pnc_j = patient_pnc[j];
//         Grad[j] += y_delta * patient_pnc_j;
//         H[j][0] += x * patient_pnc_j * y_est_1_y;
//         H[j][j] += patient_pnc_j * patient_pnc_j * y_est_1_y;

//         for (int k = 1; k < j; k += 1) {
//             H[j][k] += patient_pnc_j * patient_pnc[k] * y_est_1_y;
//         }
//     }
// }



// void Log_row::update_upperH_and_Grad(double y_est, double x, const std::vector<double>& patient_pnc) {
//     double y_est_1_y = y_est * (1 - y_est);
//     double y_delta = patient_pnc[0] - y_est;
//     Grad[0] += y_delta * x;
//     for (int j = 0; j < num_dimensions; j += 3) {
//         double pat_j_pnc0 = patient_pnc[j];
//         double pat_j_pnc1 = (j < num_dimensions) ? patient_pnc[j + 1] : 0;
//         double pat_j_pnc2 = (j < num_dimensions) ? patient_pnc[j + 2] : 0;

//         int j_is_not_0 = j != 0;
//         Grad[j] += y_delta * ((pat_j_pnc0 * j_is_not_0) + pat_j_pnc1 + pat_j_pnc2);
//         for (int k = 0; k <= j + 2; k++) {
//             double x1_0 = (j == 0) ? x : pat_j_pnc0;
//             double x1_1 = (j == 0) ? x : pat_j_pnc1;
//             double x1_2 = (j == 0) ? x : pat_j_pnc2;
//             double x2 = (k == 0) ? x : patient_pnc[k];
//             H[j][k] += x1 * x2 * y_est_1_y;
//         }
//     }
// }

// OG:

// void Log_row::update_upperH_and_Grad(double y_est, double x, const std::vector<double>& patient_pnc) {
//     double y_est_1_y = y_est * (1 - y_est);
//     double y_delta = patient_pnc[0] - y_est;
//     Grad[0] += y_delta * x;
//     for (int j = 0; j < num_dimensions; j++) {
//         if (j > 0) {
//             Grad[j] += y_delta * patient_pnc[j];
//         }
//         for (int k = 0; k <= j; k++) {
//             double x1 = (j == 0) ? x : patient_pnc[j];
//             double x2 = (k == 0) ? x : patient_pnc[k];
//             H[j][k] += x1 * x2 * y_est_1_y;
//         }
//     }
// }