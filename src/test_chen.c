#include <pbc/pbc.h>

#include "chen_ipe.h"
#include "util.h"

#include <stdio.h>
#include <string.h>


static char* param_file = NULL;


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
    //int i;
    //double keygen_time, enc_time, dec_time;
    double dec_time;
    double start, end, setup_time;

    printf("RUNNING: %s (n=%d)\n", name, dims);

    pairing_t pairing;

    size_t s_len;
    char *s = read_filename(param_file, &s_len);
    pairing_init_set_buf(pairing, s, s_len);
    free(s);

    ipe_privkey *msk;
    ipe_pubkey *mpk;
    start = get_time();
    ipe_new(dims, &pairing, &msk, &mpk);
    end = get_time();
    setup_time = end - start;

    ipe_ctx *ctx = ipe_ctx_new(mpk);

    ff_matrix *y =ff_matrix_from_array(&pairing, dims, 1, v);
    ff_matrix *x =ff_matrix_from_array(&pairing, dims, 1, attrs);

    element_t m;
    element_init_GT(m, *mpk->pairing);
    element_random(m);

    ipe_veckey *vk = ipe_veckey_gen(ctx, mpk, msk, y);
    ipe_ct *ct = ipe_encrypt(ctx, mpk, m, x);

    element_t m_dec;
    start = get_time();
    ipe_decrypt(ctx, mpk, m_dec, vk, ct);
    end = get_time();
    dec_time = end - start;

    element_printf("m:         %B\n", m);
    element_printf("decrypted: %B\n", m_dec);

    printf("setup time: %lf\n", setup_time);
    printf("decrypt time: %lf\n", dec_time);

    ipe_pubkey_free(mpk);
    ipe_privkey_free(msk);
    ipe_veckey_free(vk);
    ipe_ct_free(ct);
    ff_matrix_free(x);
    ff_matrix_free(y);
    element_clear(m);
    element_clear(m_dec);
    pairing_clear(pairing);

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


    /*
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

    int long_valid_v[256] = {1};
    int long_valid_attrs[256] = {0, 1};
    run_testcase("long_valid", 256, long_valid_v, long_valid_attrs);

    int long_inval_v[64] = {1};
    int long_inval_attrs[64] = {1};
    run_testcase("long_inval", 64, long_inval_v, long_inval_attrs);
    */

    for (int x=0; x < 10; x++) {
        int long_inval2_v[64] = {1};
        for (int i=0; i < 64; i++) long_inval2_v[i] = 1;
        int long_inval2_attrs[64] = {1};
        run_testcase("long_inval2", 64, long_inval2_v, long_inval2_attrs);
    }

    return 0;
}


