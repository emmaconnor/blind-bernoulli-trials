#include "util.h"
#include "bbt.h"


int bbt_new(int n, int ucomps, pairing_t *pairing, bbt_privkey *sk, bbt_pubkey *pk) {
    ipe_new(n, pairing, &sk->isk, &pk->ipk);
    element_init_GT(pk->m_suc, *pairing);
    element_random(pk->m_suc);
    sk->ucomps = ucomps;

    return 1;
}

void bbt_pubkey_clear(bbt_pubkey *pk) {
    ipe_pubkey_free(pk->ipk);
    element_clear(pk->m_suc);
}

void bbt_privkey_clear(bbt_privkey *sk) {
    ipe_privkey_free(sk->isk);
}


bbt_ctx *bbt_ctx_new(bbt_pubkey *pk) {
    bbt_ctx *ctx = calloc(sizeof(bbt_ctx), 1);
    pairing_t *pairing = IPE_PAIRING(pk->ipk);
    int n = IPE_DIM(pk->ipk);

    element_init_GT(ctx->m_dec, *pairing);
    ctx->v = ff_matrix_new(pairing, n, 1);
    ctx->ictx = ipe_ctx_new(pk->ipk);

    return ctx;
}


void bbt_ctx_free(bbt_ctx *ctx) {
    element_clear(ctx->m_dec);
    ff_matrix_free(ctx->v);

    ipe_ctx_free(ctx->ictx);

    free(ctx);
}

void bbt_userkey_free(bbt_userkey *uk) {
    ipe_veckey_free(uk->ivk);
    free(uk);
}

void bbt_tag_free(bbt_tag *t) {
    ipe_ct_free(t->ict);
    free(t);
}

bbt_userkey *bbt_new_userkey(bbt_ctx *ctx, bbt_privkey *sk, bbt_pubkey *pk) {
    int n = IPE_DIM(sk->isk);
    bbt_userkey *uk = calloc(sizeof(bbt_userkey), 1);

    int *indices = shuffled_range(n);

    for (int i=0; i < n; i++) {
        if (i < sk->ucomps) {
            element_set1(FF_ELEM(ctx->v, indices[i], 0));
        }
        else {
            element_set0(FF_ELEM(ctx->v, indices[i], 0));
        }
    }

    free(indices);

    uk->ivk = ipe_veckey_gen(ctx->ictx, pk->ipk, sk->isk, ctx->v);

    return uk;
}

bbt_tag *bbt_new_tag(bbt_ctx *ctx, bbt_pubkey *pk, int x) {
    int n = IPE_DIM(pk->ipk);

    bbt_tag *t = calloc(sizeof(bbt_tag), 1);

    int *indices = shuffled_range(n);

    for (int i=0; i < n; i++) {
        if (i < x) {
            element_random(FF_ELEM(ctx->v, indices[i], 0));
        }
        else {
            element_set0(FF_ELEM(ctx->v, indices[i], 0));
        }
    }

    free(indices);

    t->ict = ipe_encrypt(ctx->ictx, pk->ipk, pk->m_suc, ctx->v);

    return t;
}


int bbt_trial(bbt_ctx *ctx, bbt_userkey *uk, bbt_tag *t, bbt_pubkey *pk) {
    ipe_decrypt(ctx->ictx, pk->ipk, ctx->m_dec, uk->ivk, t->ict);

    if (element_cmp(pk->m_suc, ctx->m_dec) == 0) {
        return 0;
    }

    return 1;
}


