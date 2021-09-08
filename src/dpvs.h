#include <pbc/pbc.h>


#define FF_ELEM(F, I, J) ((F)->mat[(I)*(F)->cols + (J)])


typedef struct {
    int rows, cols;
    pairing_t *pairing; // defines the underlying finite field
    element_t *mat;
} ff_matrix;

typedef struct {
    pairing_t *pairing;
    element_t g1, g2; // generators in G1 and G2
    int N; // dimension of vector space

    ff_matrix *B1, *B2;
    ff_matrix *B1_t, *B2_t;
} dpv_space;

typedef struct {
    dpv_space *dpvs;
    element_t *vec;
} dp_vec;


dpv_space *dpvs_new(int N, pairing_t *pairing);
int dpvs_gen_bases(dpv_space *dpvs);
dpv_space *dpvs_copy_pub(dpv_space *dpvs);


ff_matrix *ff_matrix_new(pairing_t *pairing, int rows, int cols);
ff_matrix *ff_matrix_copy(ff_matrix *src);
ff_matrix *ff_matrix_rand(pairing_t *pairing, int rows, int cols);

void ff_matrix_trans(ff_matrix *A_t, ff_matrix *A);
void ff_matrix_dot(element_t dot, ff_matrix *v1, ff_matrix *v2);
void ff_matrix_block_copy(ff_matrix *dest, ff_matrix *src, int start_r, int start_c, int n_r, int n_c);


dp_vec *dpvs_rand_vec(dpv_space *dpvs, int dims, int basis);
dp_vec *dp_vec_from_coefficients(dpv_space *dpvs, ff_matrix *vec, int basis);
void dp_vec_pair(element_t r, dp_vec *v1, dp_vec *v2);
void dp_vec_print(dp_vec *v);

