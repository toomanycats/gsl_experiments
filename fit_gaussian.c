#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_errno.h>

#define DIM0 299
#define DIM1 299
#define ALPHA 5.0
#define K_SIZE 100 /* window size */
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
    gsl_vector* data_sq[DIM0];
    for (int i=0; i <= DIM0; i++)
        data_sq[i] = gsl_vector_alloc(DIM0);

    // Load Data
    int temp;
    for (int i=0; i < DIM0; i++) {
        for(int j=0; j < DIM1; j++){
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
    for (int i=0; i < DIM0; i++)
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq[i], data_sq[i], gauss_p);

    // aggreate rows and cols
	double x, mu_x, mu_old_x;
	double y, mu_y, mu_old_y;

    // final target, aggreated x and y
    gsl_vector *mux = gsl_vector_alloc(DIM0);
    gsl_vector *muy = gsl_vector_alloc(DIM1);

    for (int i=0; i < DIM0; i++){
		for (int j=0; j < DIM1; j++){
			y = gsl_vector_get(data_sq[i], j);
            x = gsl_vector_get(data_sq[j], i);

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
    for (int i=0; i < DIM0; i++) {
        x = gsl_vector_get(mux, i);
        y = gsl_vector_get(muy, i);
        printf("%f,%f\n", x, y);
    }

    // Guassian Fit
    //
    // clean up
    for (int i=0; i < DIM0; i++)
        gsl_vector_free(data_sq[i]);

    gsl_filter_gaussian_free(gauss_p);

    return 0;
}
