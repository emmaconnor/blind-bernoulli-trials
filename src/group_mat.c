#include "util.h"
#include "group_mat.h"


group_matrix *group_matrix_new(pairing_t *pairing, int rows, int cols, element_t fill) {
    group_matrix *A = calloc(sizeof(group_matrix), 1);

    if (rows < 1 || rows > MAX_MAT_DIM) {
        return NULL;
    }
    if (cols < 1 || cols > MAX_MAT_DIM) {
        return NULL;
    }

    int len = rows*cols;

    A->mat = calloc(sizeof(element_t), len);

    if (!A->mat) {
        alloc_failure();
    }

    A->pairing = pairing;
    A->rows = rows;
    A->cols = cols;

    for (int i=0; i < len; i++) {
        element_init_same_as(A->mat[i], fill);
        element_set(A->mat[i], fill);
    }

    return A;
}


void group_matrix_free(group_matrix *m) {
    int len = m->rows*m->cols;

    for (int i=0; i < len; i++) {
        element_clear(m->mat[i]);
    }
    free(m->mat);
    free(m);
}


group_matrix *group_matrix_from_ff(ff_matrix *src, element_t g) {
    group_matrix *A = calloc(sizeof(group_matrix), 1);

    int rows = src->rows;
    int cols = src->cols;
    int len = rows*cols;

    A->mat = calloc(sizeof(element_t), len);

    if (!A->mat) {
        alloc_failure();
    }

    A->pairing = src->pairing;
    A->rows = rows;
    A->cols = cols;

    for (int i=0; i < len; i++) {
        element_init_same_as(A->mat[i], g);
        element_pow_zn(A->mat[i], g, src->mat[i]);
    }

    return A;
}


void group_matrix_print(group_matrix *mat) {
    int r = mat->rows;
    int c = mat->cols;

    for (int i=0; i < r; i++) {
        element_printf("[%B,", mat->mat[i*c]);
        for (int j=1; j < c; j++) {
            element_printf(" %B,", mat->mat[i*c + j]);
        }
        printf("],\n");
    }
}


group_matrix *group_matrix_copy(group_matrix *src) {
    int rows = src->rows;
    int cols = src->cols;
    int len = rows*cols;
    pairing_t *pairing = src->pairing;


    group_matrix *A = group_matrix_new(pairing, rows, cols, src->mat[0]);

    for (int i=0; i < len; i++) {
        element_set(A->mat[i], src->mat[i]);
    }

    return A;
}


void group_matrix_block_copy(group_matrix *dest, group_matrix *src, int start_r, int start_c, int n_r, int n_c) {
    int r_max = start_r + n_r;
    int c_max = start_c + n_c;

    for (int r=start_r; r < r_max; r++) {
        for (int c=start_c; c < c_max; c++) {
            element_set(GROUP_ELEM(dest, r, c), GROUP_ELEM(src, r, c));
        }
    }
}


void group_matrix_set(group_matrix *dest, group_matrix *src) {
    group_matrix_block_copy(dest, src, 0, 0, dest->rows, dest->cols);
}

group_matrix *group_matrix_rand(pairing_t *pairing, int rows, int cols, element_t like) {
    group_matrix *A = group_matrix_new(pairing, rows, cols, like);

    int len = rows*cols;

    for (int i=0; i < len; i++) {
        element_random(A->mat[i]);
    }

    return A;
}


// need to multiply ff * group and group * ff
// pairing-based multiply

void ff_matrix_mul_group(group_matrix *R, ff_matrix *A, group_matrix *B) {
    element_t tmp, sum;
    element_init_same_as(tmp, B->mat[0]);
    element_init_same_as(sum, B->mat[0]);

    for (int i=0; i < R->rows; i++) {
        for (int j=0; j < R->cols; j++) {

            element_set1(sum);
            for (int k=0; k < A->cols; k++) {
                element_pow_zn(tmp, B->mat[k*B->cols + j], A->mat[i*A->cols + k]);
                element_mul(sum, sum, tmp);
            }

            element_set(R->mat[i*R->cols + j], sum);
        }
    }

    element_clear(tmp);
    element_clear(sum);
}

void group_matrix_mul_ff(group_matrix *R, group_matrix *A, ff_matrix *B) {
    element_t tmp, sum;
    element_init_same_as(tmp, A->mat[0]);
    element_init_same_as(sum, A->mat[0]);

    for (int i=0; i < R->rows; i++) {
        for (int j=0; j < R->cols; j++) {

            element_set1(sum);
            for (int k=0; k < A->cols; k++) {
                element_pow_zn(tmp, A->mat[i*A->cols + k], B->mat[k*B->cols + j]);
                element_mul(sum, sum, tmp);
            }

            element_set(R->mat[i*R->cols + j], sum);
        }
    }

    element_clear(tmp);
    element_clear(sum);
}

void group_matrix_pair(group_matrix *R, group_matrix *A, group_matrix *B) {
    element_t tmp, sum;
    element_init_GT(tmp, *A->pairing);
    element_init_GT(sum, *A->pairing);

    for (int i=0; i < R->rows; i++) {
        for (int j=0; j < R->cols; j++) {

            element_set1(sum);
            for (int k=0; k < A->cols; k++) {
                element_pairing(tmp, A->mat[i*A->cols + k], B->mat[k*B->cols + j]);
                element_mul(sum, sum, tmp);
            }

            element_set(R->mat[i*R->cols + j], sum);
        }
    }

    element_clear(tmp);
    element_clear(sum);
}

// TODO
void group_matrix_dot(element_t dot, group_matrix *v1, group_matrix *v2) {
    element_t tmp;
    element_init_Zr(tmp, *v1->pairing);

    element_set0(dot);
    for (int i=0; i < v1->rows; i++) {
        element_mul(tmp, v1->mat[i], v2->mat[i]);
        element_add(dot, dot, tmp);
    }

    element_clear(tmp);
}

void group_matrix_scalar_mul(group_matrix *R, element_t a, group_matrix *B) {
    /*
    double t, end, start;
    printf("start scalar mul\n");
    start = get_time();
    */
    for (int i=0; i < (B->rows * B->cols); i++) {
        //element_printf("    %B x %B\n", B->mat[i], a);
        element_pow_zn(R->mat[i], B->mat[i], a);
    }
    /*
    end = get_time();
    t = end - start;
    printf("scalar mul %d x %d matrix: %lf\n", B->rows, B->cols, t);
    */
}

void group_matrix_trans(group_matrix *A_t, group_matrix *A) {
    int r = A->rows;
    int c = A->cols;

    for (int i=0; i < r; i++) {
        for (int j=0; j < c; j++) {
            element_set(A_t->mat[j*r+i], A->mat[i*c+j]);
        }
    }
}

void group_matrix_add(group_matrix *R, group_matrix *A, group_matrix *B) {
    int r = A->rows;
    int c = A->cols;

    for (int i=0; i < r; i++) {
        for (int j=0; j < c; j++) {
            element_mul(GROUP_ELEM(R,i,j), GROUP_ELEM(A,i,j), GROUP_ELEM(B,i,j));
        }
    }
}



