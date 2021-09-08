#include <pbc/pbc.h>

#include "dpvs.h"

#include <stdio.h>


int main(int argc, char** argv) {
    double start, end, total;
    pairing_ pairing;
    pairing_init_set_str(pairing, P_PARAMS);

    int dims = 64;
    dpv_space *dpvs = dpvs_new(dims, &pairing);

    start = get_time();
    printf("generating bases (%d dimensions)...\n", dims);
    dpvs_gen_bases(dpvs);
    printf("done.\n");
    end = get_time();
    printf("basis gen time: %lf\n", end-start);

    element_t dot;

    int trials = 100;
    for (int i=0; i < trials; i++) {

        start = get_time();
        dp_vec *ku = dpvs_rand_vec(dpvs, dims/2, 1);

        element_printf("%B\n", ku->vec[0]);

        end = get_time();
        printf("ku gen time: %lf\n", end-start);

        start = get_time();
        dp_vec *t = dpvs_rand_vec(dpvs, 1, 2);
        end = get_time();
        printf("tag gen time: %lf\n", end-start);

        element_init_GT(dot, pairing);

        start = get_time();
        dp_vec_pair(dot, ku, t);
        end = get_time();

        total += (end - start);

        element_printf("ku * t = %B\n", dot);
        printf("time: %lf\n", end-start);
    }

    printf("avg time: %lf\n", total/(double)trials);

    return 0;
  
}


