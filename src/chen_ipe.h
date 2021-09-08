#include "group_mat.h"


#define IPE_DIM(MPK) ((MPK)->n)
#define IPE_PAIRING(MPK) ((MPK)->pairing)


typedef struct ipe_pubkey {
    int n; // number of dimensions in IPE keys and attrs; NOT necessarily dims in any matrix
    pairing_t *pairing;
    element_t g1, g2, gt;
    group_matrix *A_trans_1;
    group_matrix *AU_1;
    group_matrix **AW_1;
    group_matrix *Ak_t;
} ipe_pubkey;

typedef struct ipe_privkey {
    int n;
    pairing_t *pairing;
    ff_matrix *k;
    ff_matrix **W;
    ff_matrix *B_14;
} ipe_privkey;

typedef struct ipe_veckey {
    int n;
    ff_matrix *y;
    group_matrix *K0;
    group_matrix *K1;
} ipe_veckey;

typedef struct ipe_ct {
    int n;
    group_matrix *C0;
    group_matrix **Ci;
    element_t C;
} ipe_ct;


typedef struct ipe_ctx {
    group_matrix *D1;
    group_matrix *mul_tmp;
    group_matrix *pair1;
    group_matrix *pair2;
} ipe_ctx;

void ipe_new(int N, pairing_t *pairing, ipe_privkey **priv, ipe_pubkey **pub);
void ipe_pubkey_free(ipe_pubkey *mpk);
void ipe_privkey_free(ipe_privkey *msk);

ipe_ctx *ipe_ctx_new(ipe_pubkey *mpk);

ipe_veckey *ipe_veckey_gen(ipe_ctx *ctx, ipe_pubkey *mpk, ipe_privkey *msk, ff_matrix *vec);
ipe_ct *ipe_encrypt(ipe_ctx *ctx, ipe_pubkey *pk, element_t m, ff_matrix *attr);
void ipe_decrypt(ipe_ctx *ctx, ipe_pubkey *mpk, element_t m, ipe_veckey *kv, ipe_ct *ct);


void ipe_ct_free(ipe_ct *ct);
void ipe_veckey_free(ipe_veckey *vk);
void ipe_ctx_free(ipe_ctx *ctx);

