#include <pbc/pbc.h>

#include "bbt.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>


static char* param_file = NULL;
pairing_t pairing;


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




int run_testcase(int dims, int ucomps, int n_runs) {
    int i;
    double start, end, setup_time, keygen_time, taggen_time, trial_time;
    int successes = 0;

    setup_time = keygen_time = taggen_time = trial_time = 0;

    for (i=0; i < n_runs; i++) {

        bbt_pubkey pk;
        bbt_privkey sk;
        
        start = get_time();
        bbt_new(dims, ucomps, &pairing, &sk, &pk);
        end = get_time();

        bbt_ctx *ctx = bbt_ctx_new(&pk);

        setup_time += end - start;


        start = get_time();
        bbt_tag *t = bbt_new_tag(ctx, &pk, dims/2);
        end = get_time();
        taggen_time += end - start;

        start = get_time();
        bbt_userkey *uk = bbt_new_userkey(ctx, &sk, &pk);
        end = get_time();
        keygen_time += end - start;

        start = get_time();
        int r = bbt_trial(ctx, uk, t, &pk);
        end = get_time();
        trial_time += end - start;

        if (r) successes++;

        bbt_tag_free(t);
        bbt_userkey_free(uk);

        bbt_privkey_clear(&sk);
        bbt_pubkey_clear(&pk);
    }

    double p = (double)successes / (double)n_runs;
    setup_time /= n_runs;
    keygen_time /= n_runs;
    taggen_time /= n_runs;
    trial_time /= n_runs;

    printf("%-5d    %-5d    %2.6lf    %2.6lf    %2.6lf    %2.6lf    %2.6lf\n", 
            dims, ucomps, setup_time, keygen_time, taggen_time, trial_time, p);

    return 0;
}


void usage() {
    fprintf(stderr, "speed_test <param file>\n");
    exit(1);
}



int main(int argc, char** argv) {

    if (argc > 1) {
        param_file = argv[1];
    }
    else {
        usage();
    }

    size_t s_len;
    char *s = read_filename(param_file, &s_len);

    pairing_init_set_buf(pairing, s, s_len);

    printf("starting performance test with following parameters:\n");
    puts(s);

    free(s);

    printf("n        ucomps   setup       keygen      taggen      trial       p\n");
    for (int i=64; i <= 64; i+=8) {
        run_testcase(i, i/8, 10);
    }
    
    return 0;
}


