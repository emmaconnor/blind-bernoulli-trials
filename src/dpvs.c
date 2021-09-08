#include "dpvs.h"
#include "util.h"


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
        element_mul(B->mat[i], a, B->mat[i]);
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


dpv_space *dpvs_new(int N, pairing_t *pairing) {
    dpv_space *dpvs = calloc(sizeof(dpv_space), 1);

    dpvs->N = N;
    dpvs->pairing = pairing;

    element_init_G1(dpvs->g1, *pairing);
    element_init_G2(dpvs->g2, *pairing);
    element_random(dpvs->g1);

    if (pairing_is_symmetric(*pairing)) {
        element_set(dpvs->g2, dpvs->g1);
    }
    else {
        element_random(dpvs->g2);
    }

    dpvs->B1 = NULL;
    dpvs->B2 = NULL;

    return dpvs;
}


dpv_space *dpvs_copy_pub(dpv_space *dpvs) {
    dpv_space *r = calloc(sizeof(dpv_space), 1);

    r->N = dpvs->N;
    r->pairing = dpvs->pairing;

    element_init_G1(r->g1, *dpvs->pairing);
    element_init_G2(r->g2, *dpvs->pairing);
    element_set(r->g1, dpvs->g1);
    element_set(r->g2, dpvs->g2);

    r->B1 = NULL;
    r->B2 = NULL;

    return r;
}

/* 

dpvs parameters:
    - underlying pairing: defines finite field, multiplicative groups, and pairing
    - dims of vector space 

internals:
    - bases (private and public?)



   

*/

int dpvs_gen_bases(dpv_space *dpvs) {
    // TODO Phi - random scalar multiple
    int N = dpvs->N;

    ff_matrix *B1 = ff_matrix_rand(dpvs->pairing, N, N);
    ff_matrix *B1_inv = ff_matrix_copy(B1);
    //ff_matrix *R = ff_matrix_copy(B1);

    ff_matrix_inv(B1_inv, B1);

    //ff_matrix_mul(R, B1, B1_inv);

    dpvs->B1 = B1;
    dpvs->B2_t = B1_inv;

    dpvs->B1_t = ff_matrix_copy(B1);
    dpvs->B2 = ff_matrix_copy(B1);

    ff_matrix_trans(dpvs->B1_t, B1);
    ff_matrix_trans(dpvs->B2, B1_inv);

    return 1;
}


dp_vec *dp_vec_new(dpv_space *dpvs, int group) {
    int n = dpvs->N;
    dp_vec *v = calloc(sizeof(dp_vec), 1);

    v->dpvs = dpvs_copy_pub(dpvs);
    v->vec = calloc(sizeof(element_t), n);

    for (int i=0; i < n; i++) {
        if (group == 1) {
            element_init_same_as(v->vec[i], dpvs->g1);
            element_set(v->vec[i], dpvs->g1);
        }
        else {
            element_init_same_as(v->vec[i], dpvs->g2);
            element_set(v->vec[i], dpvs->g2);
        }
    }

    return v;
}

dp_vec *dpvs_rand_vec(dpv_space *dpvs, int dims, int basis) {
    ff_matrix *mat = ff_matrix_new(dpvs->pairing, dpvs->N, 1);

    int *indxs = shuffled_range(dpvs->N);
    for (int i=0; i < dims; i++) {
        element_random(mat->mat[indxs[i]]);
    }
    free(indxs);

    ff_matrix *X = dpvs->B2_t;
    if (basis == 2) {
        X = dpvs->B1_t;
    }

    ff_matrix *mat_std = ff_matrix_copy(mat);
    ff_matrix_mul(mat_std, X, mat);


    dp_vec *v = dp_vec_new(dpvs, basis);

    for (int i=0; i < dpvs->N; i++) {
        element_pow_zn(v->vec[i], v->vec[i], mat_std->mat[i]);
    }

    return v;
}


dp_vec *dp_vec_from_coefficients(dpv_space *dpvs, ff_matrix *vec, int basis) {

    ff_matrix *X = dpvs->B1;
    if (basis == 2) {
        X = dpvs->B2;
    }

    ff_matrix *canonical_exp = ff_matrix_copy(vec);
    ff_matrix_mul(canonical_exp, X, vec);

    dp_vec *v = dp_vec_new(dpvs, basis);

    for (int i=0; i < dpvs->N; i++) {
        element_pow_zn(v->vec[i], v->vec[i], canonical_exp->mat[i]);
    }

    // TODO free canonical_exp

    return v;
}


void dp_vec_print(dp_vec *v) {
    printf("[\n");
    for (int i=0; i < v->dpvs->N; i++) {
        element_printf("  %B,\n", v->vec[i]);
    }
    printf("]\n");

}


void dp_vec_pair(element_t r, dp_vec *v1, dp_vec *v2) {
    element_prod_pairing(r, v1->vec, v2->vec, v1->dpvs->N);
}

