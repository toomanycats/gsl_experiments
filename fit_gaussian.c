#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>

#define DIM 299 /* square image */
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
    gsl_vector* data_sq[DIM];
    gsl_vector* data_sq_t[DIM];

    for (int i=0; i <= DIM; i++) {
        data_sq[i] = gsl_vector_alloc(DIM);
        data_sq_t[i] = gsl_vector_alloc(DIM);
    }

    // Load Data
    int temp;
    for (int i=0; i < DIM; i++) {
        for(int j=0; j < DIM; j++){
            ret = fread(&temp, sizeof(uint16_t), 1, infile);
            if (ret < 0){
                fprintf(stderr, "fread failure.\n");
                exit(1);
            }
            gsl_vector_set(data_sq[i], j, temp);
        }
    }

    // smoothing
    gsl_filter_gaussian_workspace *gauss_p = gsl_filter_gaussian_alloc(K_SIZE);

    // iterate over array smoothing
    for (int k=0; k < DIM; k++) {
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq[k], data_sq[k], gauss_p);
    }

    // Transpose
    for (int i=0; i < DIM; i++) {
        for (int j=0; j < DIM; j++) {
			double v = gsl_vector_get(data_sq[i], j);
            gsl_vector_set(data_sq_t[j], i, v);
        }
    }

    // Smooth on transposed data
    for (int k=0; k < DIM; k++) {
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq_t[k], data_sq_t[k], gauss_p);
    }

    // Iterative Mean
	double x, mu_x, mu_old_x;
	double y, mu_y, mu_old_y;

    // final target, 1d aggregated x and y
    gsl_vector *mux = gsl_vector_alloc(DIM);
    gsl_vector *muy = gsl_vector_alloc(DIM);

    for (int i=0; i < DIM; i++){
		for (int j=0; j < DIM; j++){
			y = gsl_vector_get(data_sq_t[i], j);
            x = gsl_vector_get(data_sq_t[j], i);

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

    // Save X and Y ave to file.
    FILE *f_sm = fopen("data_sm.txt", "w");
    if (f_sm == NULL){
        printf("Could not open file for smooth data save.\n");
        exit(-1);
    }
    for (int i=0; i < DIM; i++) {
        x = gsl_vector_get(mux, i);
        y = gsl_vector_get(muy, i);
        fprintf(f_sm, "%i %f %f\n", i, x, y);
    }
    fclose(f_sm);

    // save smoothed image to file in ascii for gnuplot.
    FILE *f_img = fopen("smoothed_image.txt", "w");
    for (int i=0; i < DIM; i++) {
        for (int j=0; j < DIM; j++) {
            fprintf(f_img, "%f ", gsl_vector_get(data_sq_t[i], j));
        }
        fprintf(f_img, "\n");
    }

    // Guassian Fit
    //
    // clean up
    for (int i=0; i < DIM; i++) {
        gsl_vector_free(data_sq[i]);
        gsl_vector_free(data_sq_t[i]);
    }

    gsl_filter_gaussian_free(gauss_p);

    return 0;
}
