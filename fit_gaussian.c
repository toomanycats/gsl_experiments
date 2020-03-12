#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "fit_tools.h"

int main(){
    // do not stop program if error occurs
    //gsl_set_error_handler_off();

    // need array of vectors to create a square
    gsl_vector* data_sq[DIM];
    load_data_from_file(data_sq);
    // alloc the workspace
    gsl_filter_gaussian_workspace *gauss_p = gsl_filter_gaussian_alloc(K_SIZE);

    smooth_data_sq(data_sq);
    save_smoothed_image_to_file("smoothed_image.txt", data_sq);

    // Mean of rows and cols
    // final target, 1d aggregated x and y"
    gsl_vector *mux = gsl_vector_alloc(DIM);
    average_dim(data_sq, mux);
    gsl_vector *muy = gsl_vector_alloc(DIM);
    average_dim(data_sq, muy);
    save_averaged_to_file("data_sm.txt");

    // Guassian Fit
    fit(muy);
    fit(mux);

    // clean up
    for (int i=0; i < DIM; i++) {
        gsl_vector_free(data_sq[i]);
        gsl_vector_free(data_sq_t[i]);
    }

    gsl_filter_gaussian_free(gauss_p);

    return 0;
}
