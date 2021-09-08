#include <pbc/pbc.h>


#define FF_ELEM(F, I, J) ((F)->mat[(I)*(F)->cols + (J)])


typedef struct {
    int rows, cols;
    pairing_t *pairing; // defines the underlying finite field
    element_t *mat;
} ff_matrix;



ff_matrix *ff_matrix_new(pairing_t *pairing, int rows, int cols);
ff_matrix *ff_matrix_from_array(pairing_t *pairing, int rows, int cols, int *arr);
void ff_matrix_free(ff_matrix *mat);

ff_matrix *ff_matrix_copy(ff_matrix *src);
ff_matrix *ff_matrix_rand(pairing_t *pairing, int rows, int cols);

void ff_matrix_trans(ff_matrix *A_t, ff_matrix *A);
void ff_matrix_dot(element_t dot, ff_matrix *v1, ff_matrix *v2);
void ff_matrix_mul(ff_matrix *R, ff_matrix *A, ff_matrix *B);

void ff_matrix_scalar_mul(ff_matrix *R, element_t a, ff_matrix *B);
void ff_matrix_add(ff_matrix *R, ff_matrix *A, ff_matrix *B);



