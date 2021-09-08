#include <pbc/pbc.h>

#include "aahipe.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>


static char* param_file = NULL;


int test_enc_dec(ff_matrix *attr, ipe_veckey *kv, ipe_pubkey *pk, double *enc_time, double *dec_time) {
    double start, end;

    element_t m, dec_m;
    element_init_GT(m, *pk->dpvs->pairing);
    element_init_GT(dec_m, *pk->dpvs->pairing);

    element_random(m);

    start = get_time();
    ipe_ct *ct = ipe_encrypt(pk, m, attr);
    end = get_time();
    if (enc_time) *enc_time = end - start;

    //printf("ciphertext vector:\n");
    //dp_vec_print(ct->cv);
    //element_printf("ciphertext element: %B\n", ct->cn);

    start = get_time();
    ipe_decrypt(dec_m, kv, ct);
    end = get_time();
    if (dec_time) *dec_time = end - start;

    //element_printf("original = %B\n", m);
    //element_printf("decrypted = %B\n", dec_m);

    if (element_cmp(m, dec_m) == 0) {
        return 1;
    }

    return 0;
}


char *read_file(FILE *f, size_t *file_len) {
    size_t s_size = 1024;
    size_t s_len = 0;

    char buf[1024];

    char *s = calloc(s_size, 1);

    size_t n, new_size;

    while ((n = fread(buf, 1, sizeof(buf), f))) {
        new_size = s_size;
        while (s_len + n > new_size) {
            new_size = new_size << 1;
        }

        if (new_size != s_size) {
            s = realloc(s, new_size);
            s_size = new_size;
        }

        memcpy(s+s_len, buf, n);

        s_len += n;
    }

    if (file_len) {
        *file_len = s_len;
    }

    return s;
}


char *read_filename(char *fname, size_t *file_len) {
    FILE *f = fopen(fname, "r");
    char *s = read_file(f, file_len);
    fclose(f);
    return s;
}




int run_testcase(char *name, int dims, int v[], int attrs[]) {
    int i;
    double start, end, setup_time, keygen_time, enc_time, dec_time;

    printf("RUNNING: %s (n=%d)\n", name, dims);

    pairing_t pairing;

    size_t s_len;
    char *s = read_filename(param_file, &s_len);

    pairing_init_set_buf(pairing, s, s_len);

    free(s);

    start = get_time();
    ipe_privkey *sk = ipe_new(dims, &pairing);
    end = get_time();

    setup_time = end - start;


    ipe_pubkey *pk = ipe_get_pubkey(sk);

    ff_matrix *v_tmp = ff_matrix_new(sk->dpvs->pairing, sk->n, 1);
    for (i=0; i < dims; i++) {
        element_set_si(FF_ELEM(v_tmp, i, 0), v[i]);
    }

    start = get_time();
    ipe_veckey *kv = ipe_gen_veckey(sk, v_tmp);
    end = get_time();

    printf("number of elements: %d\n", kv->kv->dpvs->N);

    keygen_time = end - start;

    ff_matrix *attrs_tmp = ff_matrix_new(pk->dpvs->pairing, pk->n, 1);
    for (i=0; i < dims; i++) {
        element_set_si(FF_ELEM(attrs_tmp, i, 0), attrs[i]);
    }

    int should_succeed = 1;
    for (i=0; i < dims; i++) {
        if (attrs[i] && v[i]) {
            should_succeed = 0;
            break;
        }
    }

    int r = test_enc_dec(attrs_tmp, kv, pk, &enc_time, &dec_time) ? 1 : 0;

    printf("Decryption result: %s\n", r ? "success" : "fail");
    printf("Expected result:   %s\n", should_succeed ? "success" : "fail");

    printf("    setup   keygen  enc     dec\n");
    printf("    %3.3lf %3.3lf %3.3lf %3.3lf\n", setup_time, keygen_time, enc_time, dec_time);
    printf("RESULT: %s\n", r == should_succeed ? "PASS" : "FAIL");

    if (r == should_succeed) {
        return 1;
    }

    return 0;
}


void usage() {
    fprintf(stderr, "test_ipe <param file>\n");
    exit(1);
}



int main(int argc, char** argv) {

    if (argc > 1) {
        param_file = argv[1];
    }
    else {
        usage();
    }



    int simple_valid_v[] = {1, 0, 0};
    int simple_valid_attrs[] = {0, 0, 1};
    run_testcase("simple_valid", 3, simple_valid_v, simple_valid_attrs);

    int simple_inval_v[] = {1, 0, 0};
    int simple_inval_attrs[] = {1, 0, 0};
    run_testcase("simple_inval", 3, simple_inval_v, simple_inval_attrs);

    int med_val_v[16] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    int med_val_attrs[16] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    run_testcase("med_val", 16, med_val_v, med_val_attrs);

    int med_inval_v[16] =     {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1};
    int med_inval_attrs[16] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    run_testcase("med_inval", 16, med_inval_v, med_inval_attrs);

    int long_valid_v[64] = {1};
    int long_valid_attrs[64] = {0, 1};
    run_testcase("long_valid", 64, long_valid_v, long_valid_attrs);

    int long_inval_v[64] = {1};
    int long_inval_attrs[64] = {1};
    run_testcase("long_inval", 64, long_inval_v, long_inval_attrs);

    return 0;
}


