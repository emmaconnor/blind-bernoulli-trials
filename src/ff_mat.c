#include "util.h"
#include "ff_mat.h"


ff_matrix *ff_matrix_new(pairing_t *pairing, int rows, int cols) {
    ff_matrix *A = calloc(sizeof(ff_matrix), 1);
    int len = rows*cols;

    // todo check for overflow
    A->mat = calloc(sizeof(element_t), len);
    A->pairing = pairing;
    A->rows = rows;
    A->cols = cols;

    for (int i=0; i < len; i++) {
        element_init_Zr(A->mat[i], *pairing);
        element_set0(A->mat[i]);
    }

    return A;
}

ff_matrix *ff_matrix_from_array(pairing_t *pairing, int rows, int cols, int *arr) {
    ff_matrix *A = calloc(sizeof(ff_matrix), 1);
    int len = rows*cols;

    // todo check for overflow
    A->mat = calloc(sizeof(element_t), len);
    A->pairing = pairing;
    A->rows = rows;
    A->cols = cols;

    for (int i=0; i < len; i++) {
        element_init_Zr(A->mat[i], *pairing);
        element_set_si(A->mat[i], arr[i]);
    }

    return A;
}


void ff_matrix_free(ff_matrix *m) {
    int len = m->rows*m->cols;

    for (int i=0; i < len; i++) {
        element_clear(m->mat[i]);
    }
    free(m->mat);
    free(m);
}


void ff_matrix_print(ff_matrix *mat) {
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


ff_matrix *ff_matrix_copy(ff_matrix *src) {
    int rows = src->rows;
    int cols = src->cols;
    int len = rows*cols;
    pairing_t *pairing = src->pairing;

    ff_matrix *A = ff_matrix_new(pairing, rows, cols);

    for (int i=0; i < len; i++) {
        element_set(A->mat[i], src->mat[i]);
    }

    return A;
}


void ff_matrix_block_copy(ff_matrix *dest, ff_matrix *src, int start_r, int start_c, int n_r, int n_c) {
    int r_max = start_r + n_r;
    int c_max = start_c + n_c;

    for (int r=start_r; r < r_max; r++) {
        for (int c=start_c; c < c_max; c++) {
            element_set(FF_ELEM(dest, r, c), FF_ELEM(src, r, c));
        }
    }
}


ff_matrix *ff_matrix_rand(pairing_t *pairing, int rows, int cols) {
    ff_matrix *A = ff_matrix_new(pairing, rows, cols);

    int len = rows*cols;

    for (int i=0; i < len; i++) {
        element_random(A->mat[i]);
    }

    return A;
}


void r1_pluseq_ar2(int r1, int r2, element_t a, ff_matrix *A, ff_matrix *Ainv) {
    int n = Ainv->cols;
    element_t tmp;
    element_init_Zr(tmp, *Ainv->pairing);
    for (int i=0; i < n; i++) {
        if (A) {
            element_mul(tmp, A->mat[r2*n+i], a);
            element_add(A->mat[r1*n+i], A->mat[r1*n+i], tmp);
        }

        element_mul(tmp, Ainv->mat[r2*n+i], a);
        element_add(Ainv->mat[r1*n+i], Ainv->mat[r1*n+i], tmp);
    }
    element_clear(tmp);
}


void row_swap(int r1, int r2, ff_matrix *A, ff_matrix *Ainv) {
    int n = A->cols;
    element_t tmp;
    element_init_Zr(tmp, *A->pairing);
    for (int i=0; i < n; i++) {
        element_set(tmp, A->mat[r1*n + i]);
        element_set(A->mat[r1*n + i], A->mat[r2*n + i]);
        element_set(A->mat[r2*n + i], tmp);

        element_set(tmp, Ainv->mat[r1*n + i]);
        element_set(Ainv->mat[r1*n + i], Ainv->mat[r2*n + i]);
        element_set(Ainv->mat[r2*n + i], tmp);
    }
    element_clear(tmp);
}


int ff_matrix_inv(ff_matrix *Ainv, ff_matrix *A) {
    A = ff_matrix_copy(A);
    pairing_t *pairing = A->pairing;
    int n = A->rows;

    // set Ainv to identity
    for (int i=0; i < n; i++) {
        for (int j=0; j < n; j++) {
            if (i == j) {
                element_set1(Ainv->mat[i*n + j]);
            }
            else {
                element_set0(Ainv->mat[i*n + j]);
            }
        }
    }

    element_t inv, neg;
    element_init_Zr(inv, *pairing);
    element_init_Zr(neg, *pairing);
    for (int pivot=0; pivot < n; pivot++) {
        int nz_row = -1;
        for (int i=pivot; i < n; i++) {
            if (!element_is0(A->mat[i*n+pivot])) {
                nz_row = i;
                break;
            }
        }

        if (nz_row < 0) return -1;
        if (nz_row != pivot) {
            row_swap(pivot, nz_row, A, Ainv);
        }

        element_invert(inv, A->mat[pivot*n+pivot]);

        for (int i=0; i < n; i++) {
            element_mul(A->mat[pivot*n+i], A->mat[pivot*n+i], inv);
            element_mul(Ainv->mat[pivot*n+i], Ainv->mat[pivot*n+i], inv);
        }

        for (int cancel=pivot+1; cancel < n; cancel++) {
            element_neg(neg, A->mat[cancel*n + pivot]);

            r1_pluseq_ar2(cancel, pivot, neg, A, Ainv);
        }
    }


    for (int pivot=n-1; pivot >= 0; pivot--) {
        for (int cancel=0; cancel < pivot; cancel++) {
            element_neg(neg, A->mat[cancel*n + pivot]);

            r1_pluseq_ar2(cancel, pivot, neg, A, Ainv);
        }
    }

    return 0;
}

void ff_matrix_mul(ff_matrix *R, ff_matrix *A, ff_matrix *B) {
    element_t tmp, sum;
    element_init_Zr(tmp, *A->pairing);
    element_init_Zr(sum, *A->pairing);

    for (int i=0; i < R->rows; i++) {
        for (int j=0; j < R->cols; j++) {

            element_set0(sum);
            for (int k=0; k < A->cols; k++) {
                element_mul(tmp, A->mat[i*A->cols + k], B->mat[k*B->cols + j]);
                element_add(sum, sum, tmp);
            }

            element_set(R->mat[i*R->cols + j], sum);
        }
    }

    element_clear(tmp);
    element_clear(sum);
}

void ff_matrix_dot(element_t dot, ff_matrix *v1, ff_matrix *v2) {
    element_t tmp;
    element_init_Zr(tmp, *v1->pairing);

    element_set0(dot);
    for (int i=0; i < v1->rows; i++) {
        element_mul(tmp, v1->mat[i], v2->mat[i]);
        element_add(dot, dot, tmp);
    }

    element_clear(tmp);
}

void ff_matrix_scalar_mul(ff_matrix *R, element_t a, ff_matrix *B) {
    for (int i=0; i < (B->rows * B->cols); i++) {
        element_mul(R->mat[i], a, B->mat[i]);
    }
}

void ff_matrix_trans(ff_matrix *A_t, ff_matrix *A) {
    int r = A->rows;
    int c = A->cols;

    for (int i=0; i < r; i++) {
        for (int j=0; j < c; j++) {
            element_set(A_t->mat[j*r+i], A->mat[i*c+j]);
        }
    }
}


void ff_matrix_add(ff_matrix *R, ff_matrix *A, ff_matrix *B) {
    int r = A->rows;
    int c = A->cols;

    for (int i=0; i < r; i++) {
        for (int j=0; j < c; j++) {
            element_add(FF_ELEM(R,i,j), FF_ELEM(A,i,j), FF_ELEM(B,i,j));
        }
    }
}


