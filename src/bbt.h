#include "chen_ipe.h"
#include "util.h"

typedef struct bbt_privkey {
    ipe_privkey *isk;
    int ucomps;
} bbt_privkey;

typedef struct bbt_pubkey {
    ipe_pubkey *ipk;
    element_t m_suc;
} bbt_pubkey;

typedef struct bbt_userkey {
    ipe_veckey *ivk;
} bbt_userkey;

typedef struct bbt_tag {
    ipe_ct *ict;
} bbt_tag;

typedef struct bbt_ctx {
    ff_matrix *v;
    element_t m_dec;
    ipe_ctx *ictx;
} bbt_ctx;


int bbt_new(int n, int ucomps, pairing_t *pairing, bbt_privkey *sk, bbt_pubkey *pk);
bbt_ctx *bbt_ctx_new(bbt_pubkey *pk);
void bbt_pubkey_clear(bbt_pubkey *pk);
void bbt_privkey_clear(bbt_privkey *sk);

void bbt_userkey_free(bbt_userkey *uk);
void bbt_tag_free(bbt_tag *t);


bbt_userkey *bbt_new_userkey(bbt_ctx *ctx, bbt_privkey *sk, bbt_pubkey *pk);
bbt_tag *bbt_new_tag(bbt_ctx *ctx, bbt_pubkey *pk, int x);
int bbt_trial(bbt_ctx *ctx, bbt_userkey *uk, bbt_tag *t, bbt_pubkey *pk);
