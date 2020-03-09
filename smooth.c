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

    /* testing read
    for (int i=0; i<DIM0; i++){
        long unsigned int x = gsl_vector_get(data_sq[150], i);
        printf("%lu\n", x);
    }
    */

    // smoothing
    gsl_filter_gaussian_workspace *gauss_p = gsl_filter_gaussian_alloc(K_SIZE);
    // kernel
    gsl_vector *kernel = gsl_vector_alloc(K_SIZE);
    gsl_filter_gaussian_kernel(ALPHA, 0, 0, kernel);
    // iterate over array smoothing
    for (int i=0; i < DIM0; i++)
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq[i], data_sq[i], gauss_p);

    // aggreate rows and cols
	double x, mu, mu_old;
    for (int i=0; i < DIM0; i++){
		for (int j=0; j < DIM1; j++){
			x = gsl_vector_get(data_sq[i], j);
            if (j == 0)
                mu_old = x;
            mu = mu_old + (x - mu_old) / (double)(j + 1);
            mu_old = mu;
		}
        //testing
        printf("%f\n", mu);
    }

    // clean up
    for (int i=0; i < DIM0; i++)
        gsl_vector_free(data_sq[i]);

    gsl_filter_gaussian_free(gauss_p);

    return 0;
}
