#include "chen_ipe.h"
#include "util.h"

ipe_ctx *ipe_ctx_new(ipe_pubkey *mpk) {
    ipe_ctx *ctx = calloc(sizeof(ipe_ctx), 1);
    ctx->mul_tmp = group_matrix_new(mpk->pairing, 1, 4, mpk->g1);
    ctx->D1 = group_matrix_copy(ctx->mul_tmp);

    ctx->pair1 = group_matrix_new(mpk->pairing, 1, 1, mpk->gt);
    ctx->pair2 = group_matrix_new(mpk->pairing, 1, 1, mpk->gt);

    return ctx;
}

void ipe_ctx_free(ipe_ctx *ctx) {
    group_matrix_free(ctx->mul_tmp);
    group_matrix_free(ctx->D1);
    group_matrix_free(ctx->pair1);
    group_matrix_free(ctx->pair2);

    free(ctx);
}



void ipe_new(int N, pairing_t *pairing, ipe_privkey **priv, ipe_pubkey **pub) {
    int i;
    ipe_privkey *msk = calloc(sizeof(ipe_privkey), 1);
    ipe_pubkey *mpk = calloc(sizeof(ipe_pubkey), 1);

    msk->n = N;
    mpk->n = N;
    mpk->pairing = pairing;
    msk->pairing = pairing;

    element_init_G1(mpk->g1, *pairing);
    element_init_G2(mpk->g2, *pairing);
    element_init_GT(mpk->gt, *pairing);

    element_random(mpk->g1);
    element_random(mpk->g2);
    element_pairing(mpk->gt, mpk->g1, mpk->g2);

    ff_matrix *A_trans = ff_matrix_rand(pairing, 2, 3); // freed
    ff_matrix *B_14 = ff_matrix_rand(pairing, 4, 2); // used
    ff_matrix *U = ff_matrix_rand(pairing, 3, 4); // freed
    ff_matrix **W = calloc(sizeof(ff_matrix*), N); // used 
    ff_matrix *k = ff_matrix_rand(pairing, 3, 1); // used

    for (i=0; i < N; i++) {
        W[i] = ff_matrix_rand(pairing, 3, 4);
    }

    msk->k = k;
    msk->W = W;
    msk->B_14 = B_14;

    mpk->A_trans_1 = group_matrix_from_ff(A_trans, mpk->g1);

    // used for multiplying AW and AU
    ff_matrix *tmp = ff_matrix_new(pairing, 2, 4); // freed

    ff_matrix_mul(tmp, A_trans, U);
    mpk->AU_1 = group_matrix_from_ff(tmp, mpk->g1);

    mpk->AW_1 = calloc(sizeof(group_matrix*), N);
    for (i=0; i < N; i++) {
        ff_matrix_mul(tmp, A_trans, W[i]);
        mpk->AW_1[i] = group_matrix_from_ff(tmp, mpk->g1);
    }

    ff_matrix *Ak = ff_matrix_new(pairing, 2, 1); // freed
    ff_matrix_mul(Ak, A_trans, k);
    mpk->Ak_t = group_matrix_from_ff(Ak, mpk->gt);

    ff_matrix_free(tmp);
    ff_matrix_free(A_trans);
    ff_matrix_free(U);
    ff_matrix_free(Ak);

    *priv = msk;
    *pub = mpk;
}

void ipe_pubkey_free(ipe_pubkey *mpk) { 
    element_clear(mpk->g1);
    element_clear(mpk->g2);
    element_clear(mpk->gt);
    group_matrix_free(mpk->A_trans_1);
    group_matrix_free(mpk->AU_1);
    for (int i=0; i < mpk->n; i++) {
        group_matrix_free(mpk->AW_1[i]);
    }
    free(mpk->AW_1);
    group_matrix_free(mpk->Ak_t);
    free(mpk);
}

void ipe_privkey_free(ipe_privkey *msk) {
    ff_matrix_free(msk->k);
    for (int i=0; i < msk->n; i++) {
        ff_matrix_free(msk->W[i]);
    }
    free(msk->W);
    ff_matrix_free(msk->B_14);
    free(msk);
}

ipe_veckey *ipe_veckey_gen(ipe_ctx *ctx, ipe_pubkey *mpk, ipe_privkey *msk, ff_matrix *vec) {
    int i;
    ipe_veckey *vk = calloc(sizeof(ipe_veckey), 1);
    vk->n = mpk->n;
    vk->y = ff_matrix_copy(vec);

    ff_matrix *r = ff_matrix_rand(mpk->pairing, 2, 1);

    ff_matrix *sum = ff_matrix_new(mpk->pairing, 3, 4);
    ff_matrix *tmp = ff_matrix_new(mpk->pairing, 3, 4);

    for (i=0; i < mpk->n; i++) {
        // OPTIMIZE check for 0
        ff_matrix_scalar_mul(tmp, FF_ELEM(vec, i, 0), msk->W[i]);
        ff_matrix_add(sum, sum, tmp);
    }

    ff_matrix *WiB = ff_matrix_new(mpk->pairing, 3, 2);
    ff_matrix_mul(WiB, sum, msk->B_14);

    ff_matrix *K0 = ff_matrix_new(mpk->pairing, 3, 1);
    ff_matrix_mul(K0, WiB, r);
    ff_matrix_add(K0, msk->k, K0);

    vk->K0 = group_matrix_from_ff(K0, mpk->g2);

    ff_matrix *K1 = ff_matrix_new(mpk->pairing, 4, 1);
    ff_matrix_mul(K1, msk->B_14, r);
    vk->K1 = group_matrix_from_ff(K1, mpk->g2);

    // globals
    ff_matrix_free(r);
    ff_matrix_free(sum);
    ff_matrix_free(tmp);
    ff_matrix_free(WiB);
    ff_matrix_free(K0);
    ff_matrix_free(K1);

    return vk;
}

void ipe_veckey_free(ipe_veckey *vk) {
    ff_matrix_free(vk->y);
    group_matrix_free(vk->K0);
    group_matrix_free(vk->K1);
    free(vk);
}

/*
   typedef struct ipe_ct {
    int n;
    group_matrix *C0;
    group_matrix **C;
    element_t C;
} ipe_ct;

   */

ipe_ct *ipe_encrypt(ipe_ctx *ctx, ipe_pubkey *mpk, element_t m, ff_matrix *attr) {
    int i;
    ipe_ct *ct = calloc(sizeof(ipe_ct), 1);
    ct->n = mpk->n;
    ff_matrix *s_trans = ff_matrix_rand(mpk->pairing, 1, 2);

    ct->C0 = group_matrix_new(mpk->pairing, 1, 3, mpk->g1);
    ff_matrix_mul_group(ct->C0, s_trans, mpk->A_trans_1);

    ff_matrix *xs = ff_matrix_copy(s_trans);
    group_matrix *xsAU = group_matrix_new(mpk->pairing, 1, 4, mpk->g1);
    group_matrix *sAW = group_matrix_new(mpk->pairing, 1, 4, mpk->g1);

    ct->Ci = calloc(sizeof(group_matrix*), mpk->n);
    for (i=0; i < mpk->n; i++) {
        ct->Ci[i] = group_matrix_new(mpk->pairing, 1, 4, mpk->g1);
        ff_matrix_scalar_mul(xs, FF_ELEM(attr,i,0), s_trans);
        ff_matrix_mul_group(xsAU, xs, mpk->AU_1);

        ff_matrix_mul_group(ct->Ci[i], s_trans, mpk->AW_1[i]);

        group_matrix_add(ct->Ci[i], ct->Ci[i], xsAU);
    }

    group_matrix *C = group_matrix_new(mpk->pairing, 1, 1, mpk->gt);
    ff_matrix_mul_group(C, s_trans, mpk->Ak_t);
    element_init_GT(ct->C, *mpk->pairing);
    element_mul(ct->C, GROUP_ELEM(C,0,0), m);

    group_matrix_free(C);
    group_matrix_free(xsAU);
    group_matrix_free(sAW);
    ff_matrix_free(xs);
    ff_matrix_free(s_trans);

    return ct;
}

void ipe_ct_free(ipe_ct *ct) {
    group_matrix_free(ct->C0);
    for (int i=0; i < ct->n; i++) {
        group_matrix_free(ct->Ci[i]);
    }
    free(ct->Ci);
    element_clear(ct->C);
    free(ct);
}

void ipe_decrypt(ipe_ctx *ctx, ipe_pubkey *mpk, element_t m, ipe_veckey *kv, ipe_ct *ct) {
    int i;

    for (i=0; i < kv->n; i++) {
        group_matrix_scalar_mul(ctx->mul_tmp, FF_ELEM(kv->y,i,0), ct->Ci[i]);

        if (i == 0) {
            group_matrix_set(ctx->D1, ctx->mul_tmp);
        }
        else {
            group_matrix_add(ctx->D1, ctx->D1, ctx->mul_tmp);
        }

    }

    group_matrix_pair(ctx->pair1, ctx->D1, kv->K1);

    group_matrix_pair(ctx->pair2, ct->C0, kv->K0);

    element_init_same_as(m, ct->C);
    element_set(m, ct->C);
    element_mul(m, m, GROUP_ELEM(ctx->pair1,0,0));
    element_div(m, m, GROUP_ELEM(ctx->pair2,0,0));

}

