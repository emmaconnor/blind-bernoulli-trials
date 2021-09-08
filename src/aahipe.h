#include "dpvs.h"


typedef struct ipe_pubkey {
    int n; // number of dimensions in IPE keys and attrs; NOT necessarily dims in underlying DPVS
    dpv_space *dpvs;
} ipe_pubkey;

typedef struct ipe_privkey {
    int n;
    dpv_space *dpvs;
} ipe_privkey;

typedef struct ipe_veckey {
    int n;
    dpv_space *dpvs;
    dp_vec *kv;
} ipe_veckey;

typedef struct ipe_ct {
    int n;
    dpv_space *dpvs;
    dp_vec *cv;
    element_t cn;
} ipe_ct;

ipe_privkey *ipe_new(int N, pairing_t *pairing);

ipe_pubkey *ipe_get_pubkey(ipe_privkey *sk);

ipe_ct *ipe_encrypt(ipe_pubkey *pk, element_t m, ff_matrix *attr);
void ipe_decrypt(element_t m, ipe_veckey *kv, ipe_ct *ct);
ipe_veckey *ipe_gen_veckey(ipe_privkey *sk, ff_matrix *v);


