#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>
#include "fit_tools.h"

int main(){
    // do not stop program if error occurs
    //gsl_set_error_handler_off();

    // need array of vectors to create a square
    gsl_vector* data_sq[DIM];
    load_data_from_file(data_sq);
    // alloc the workspace
    gsl_filter_gaussian_workspace *gauss_p = gsl_filter_gaussian_alloc(K_SIZE);

    smooth_data_sq(data_sq, gauss_p);
    char *sm_image_outfile = "smoothed_image.txt";
    save_smoothed_image(sm_image_outfile, data_sq);

    // Mean of rows and cols
    // final target, 1d aggregated x and y"
    gsl_vector *mux = gsl_vector_alloc(DIM);
    int axis = 0;
    average_dim(data_sq, mux, axis);

    // Y dim
    gsl_vector *muy = gsl_vector_alloc(DIM);
    axis = 1;
    average_dim(data_sq, muy, axis);

    // save both x and y averages
    char *data_sm_outfile = "data_sm.txt";
    save_averaged_to_file(data_sm_outfile, mux, muy);

    // Guassian Fit
    FinalPos *fpx = malloc(sizeof(FinalPos));
    Data *fit_data_x = malloc(sizeof(Data));
    fit_data_x->t = malloc(DIM * sizeof(double));
    fit_data_x->y = malloc(DIM * sizeof(double));
    fit_data_x->n = DIM;
    fit(mux, fpx, fit_data_x);

    //save ascii data to file
    char *outfile_data_model_x = "data_model_x.txt";
    save_data_and_model(outfile_data_model_x, fpx, fit_data_x);

    FinalPos *fpy = malloc(sizeof(FinalPos));
    Data *fit_data_y = malloc(sizeof(Data));
    fit_data_y->t = malloc(DIM * sizeof(double));
    fit_data_y->y = malloc(DIM * sizeof(double));
    fit_data_y->n = DIM;
    fit(muy, fpy, fit_data_y);

    char *outfile_data_model_y = "data_model_y.txt";
    save_data_and_model(outfile_data_model_y, fpy, fit_data_y);

    // clean up
    for (int i=0; i < DIM; i++) {
        gsl_vector_free(data_sq[i]);
    }

    gsl_vector_free(mux);
    gsl_vector_free(muy);
    gsl_filter_gaussian_free(gauss_p);
    free(fpx);
    free(fpy);
    free(fit_data_x);
    free(fit_data_y);

    return 0;
}
