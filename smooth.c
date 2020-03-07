#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>

#define DIM0 299
#define DIM1 299
#define ALPHA 0.5
#define K_SIZE 5
#define INFILE "data.bin"

int main(){

    int ret_val;
    const double alpha = ALPHA;
    const size_t K_size = K_SIZE;

    FILE *infile;
    infile = fopen(INFILE, "rb");

    gsl_vector *kernel = gsl_vector_alloc(K_size);
    //gsl_filter_gaussian_workspace *gauss_workspace = gsl_filter_gaussian_allocate(K_size);

    // need array of vectors to create a square
    gsl_vector* data_sq[DIM0];
    for (int i=0; i < DIM1; i++)
        data_sq[i] = gsl_vector_alloc(DIM0 * sizeof(int));

    // Load Data
    for (int i=0; i < DIM0; i++) {
        // move along the 1 dim file 299 points at a time
        fseek(infile, i * DIM0, SEEK_SET);
        ret_val = gsl_vector_fread(infile, data_sq[i]);
        if (ret_val != 0){
            printf("Failed to read data into vector.");
            exit(-1);
        }
    }

    // iterate over marray, smoothing
    return 0;
}

/*
 * gcc -o myprog -lgslcblas -lgsl
 */
