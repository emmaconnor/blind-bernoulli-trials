#include <pbc/pbc.h>
#include "ff_mat.h"


#define MAX_MAT_DIM 0xffff
#define GROUP_ELEM(M, I, J) ((M)->mat[(I)*(M)->cols + (J)])


typedef struct {
    int rows, cols;
    pairing_t *pairing; // defines the underlying finite field
    element_t *mat;
} group_matrix;


group_matrix *group_matrix_new(pairing_t *pairing, int rows, int cols, element_t fill);
void group_matrix_free(group_matrix *m);

void group_matrix_print(group_matrix *mat);

group_matrix *group_matrix_copy(group_matrix *src);

group_matrix *group_matrix_rand(pairing_t *pairing, int rows, int cols, element_t like);

void ff_matrix_mul_group(group_matrix *R, ff_matrix *A, group_matrix *B);

void group_matrix_mul_ff(group_matrix *R, group_matrix *A, ff_matrix *B);

void group_matrix_pair(group_matrix *R, group_matrix *A, group_matrix *B);

void group_matrix_scalar_mul(group_matrix *R, element_t a, group_matrix *B);

void group_matrix_trans(group_matrix *A_t, group_matrix *A);

group_matrix *group_matrix_from_ff(ff_matrix *src, element_t g);

void group_matrix_add(group_matrix *R, group_matrix *A, group_matrix *B);

void group_matrix_set(group_matrix *dest, group_matrix *src);



