#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>

#define DIM0 299
#define DIM1 299
#define ALPHA 10.0
#define K_SIZE 51 /* window size */
#define INFILE "data.bin"

int main(){

    int ret;
    FILE *infile;
    infile = fopen(INFILE, "rb");
    if (infile == NULL){
        printf("did not open file.\n");
        exit(-1);
    }

    // do not stop program if error occurs
    //gsl_set_error_handler_off();

    // need array of vectors to create a square
    gsl_vector* data_sq0[DIM0];
    gsl_vector* data_sq1[DIM1];

    for (int i=0; i <= DIM0; i++) {
        data_sq0[i] = gsl_vector_alloc(DIM0);
        data_sq1[i] = gsl_vector_alloc(DIM1);
    }

    // Load Data
    int temp;
    for (int i=0; i < DIM0; i++) {
        for(int j=0; j < DIM1; j++){
            ret = fread(&temp, sizeof(uint16_t), 1, infile);
            if (ret < 0){
                fprintf(stderr, "fread failure.\n");
                exit(1);
            }
            gsl_vector_set(data_sq0[i], j, temp);
            gsl_vector_set(data_sq1[j], i, temp);
        }
    }

    // smoothing
    gsl_filter_gaussian_workspace *gauss_p = gsl_filter_gaussian_alloc(K_SIZE);
    // iterate over array smoothing
    for (int k=0; k < DIM0; k++) {
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq0[k], data_sq0[k], gauss_p);
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq1[k], data_sq1[k], gauss_p);
    }

    // Iterative Mean
	double x, mu_x, mu_old_x;
	double y, mu_y, mu_old_y;

    // final target, 1d aggregated x and y
    gsl_vector *mux = gsl_vector_alloc(DIM0);
    gsl_vector *muy = gsl_vector_alloc(DIM1);

    for (int i=0; i < DIM0; i++){
		for (int j=0; j < DIM1; j++){
			y = gsl_vector_get(data_sq0[i], j);
            x = gsl_vector_get(data_sq1[j], i);

            if (j == 0) {
                mu_old_x = x;
                mu_old_y = y;
            }

            mu_x = mu_old_x + (x - mu_old_x) / (double)(j + 1);
            mu_old_x = mu_x;

            mu_y = mu_old_y + (y - mu_old_y) / (double)(j + 1);
            mu_old_y = mu_y;
		}
        gsl_vector_set(muy, i, mu_y);
        gsl_vector_set(mux, i, mu_x);
    }

    // print for testing
    FILE *f_sm = fopen("data_sm.txt", "w");
    if (f_sm == NULL){
        printf("Could not open file for smooth data save.\n");
        exit(-1);
    }
    for (int i=0; i < DIM0; i++) {
        x = gsl_vector_get(mux, i);
        y = gsl_vector_get(muy, i);
        fprintf(f_sm, "%i %f %f\n", i, x, y);
    }

    // Guassian Fit
    //
    // clean up
    for (int i=0; i < DIM0; i++) {
        gsl_vector_free(data_sq0[i]);
        gsl_vector_free(data_sq1[i]);
    }

    gsl_filter_gaussian_free(gauss_p);

    return 0;
}