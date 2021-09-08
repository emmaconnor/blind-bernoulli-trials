#include "aahipe.h"


ipe_privkey *ipe_new(int N, pairing_t *pairing) {
    ipe_privkey *sk = calloc(sizeof(ipe_privkey), 1);

    sk->n = N;
    sk->dpvs = dpvs_new(4*N+2, pairing);
    dpvs_gen_bases(sk->dpvs);

    return sk;
}


void copy_public_bases(dpv_space *dest, dpv_space *src, int n) {
    ff_matrix *B1 = ff_matrix_new(dest->pairing, dest->N, dest->N);
    ff_matrix *B1_t = ff_matrix_copy(B1);

    ff_matrix_block_copy(B1, src->B1, 0, 0, dest->N, n + 1);
    ff_matrix_block_copy(B1, src->B1, 0, 4*n+1, dest->N, 1);

    ff_matrix_trans(B1_t, B1);

    dest->B1 = B1;
    dest->B1_t = B1_t;
}


ipe_pubkey *ipe_get_pubkey(ipe_privkey *sk) {
    ipe_pubkey *pk = calloc(sizeof(ipe_pubkey), 1);

    pk->dpvs = dpvs_copy_pub(sk->dpvs);
    pk->n = sk->n;

    copy_public_bases(pk->dpvs, sk->dpvs, pk->n);

    return pk;
}


ipe_veckey *ipe_gen_veckey(ipe_privkey *sk, ff_matrix *v) {
    int i;

    ff_matrix *eta = ff_matrix_rand(sk->dpvs->pairing, sk->n, 1);

    element_t sigma;
    element_init_Zr(sigma, *sk->dpvs->pairing);
    element_random(sigma);

    ff_matrix *coefs = ff_matrix_new(sk->dpvs->pairing, sk->dpvs->N, 1);

    element_set1(FF_ELEM(coefs, 0, 0));

    for (i=0; i < sk->n; i++) {
        //element_printf("kv[%d] = %B * %B\n", i+1, sigma, FF_ELEM(v, i, 0));
        element_mul(FF_ELEM(coefs, i+1, 0), sigma, FF_ELEM(v, i, 0));
    }

    for (i=0; i < sk->n; i++) {
        element_set(FF_ELEM(coefs, 1+3*sk->n+i, 0), FF_ELEM(eta, i, 0));
    }


    ipe_veckey *r = calloc(sizeof(ipe_veckey), 1);

    r->kv = dp_vec_from_coefficients(sk->dpvs, coefs, 2);
    r->dpvs = dpvs_copy_pub(sk->dpvs);
    r->n = sk->n;

    return r;
}


ipe_ct *ipe_encrypt(ipe_pubkey *pk, element_t m, ff_matrix *attr) {
    dpv_space *dpvs = pk->dpvs;
    element_t omega;
    element_init_Zr(omega, *dpvs->pairing);
    element_random(omega);

    ff_matrix *coefs = ff_matrix_new(dpvs->pairing, dpvs->N, 1);

    element_random(FF_ELEM(coefs, 0, 0));
    element_random(FF_ELEM(coefs, 4*pk->n+1, 0));

    for (int i=0; i < pk->n; i++) {
        //element_printf("ct[%d] = %B * %B\n", i+1, omega, FF_ELEM(attr, i, 0));
        element_mul(FF_ELEM(coefs, i+1, 0), omega, FF_ELEM(attr, i, 0));
    }

    ipe_ct *ct = calloc(sizeof(ipe_ct), 1);
    ct->n = pk->n;
    ct->dpvs = dpvs_copy_pub(dpvs);
    ct->cv = dp_vec_from_coefficients(dpvs, coefs, 1);

    element_t gt, gt_eta;

    element_init_GT(gt, *dpvs->pairing);
    element_init_GT(gt_eta, *dpvs->pairing);
    element_init_GT(ct->cn, *dpvs->pairing);

    pairing_apply(gt, dpvs->g1, dpvs->g2, *dpvs->pairing);

    element_pow_zn(gt_eta, gt, FF_ELEM(coefs, 0, 0));
    element_add(ct->cn, gt_eta, m);

    return ct;
}


void ipe_decrypt(element_t m, ipe_veckey *kv, ipe_ct *ct) {
    element_t gt_eta;
    element_init_GT(gt_eta, *kv->dpvs->pairing);

    dp_vec_pair(gt_eta, ct->cv, kv->kv);

    element_sub(m, ct->cn, gt_eta);
}


