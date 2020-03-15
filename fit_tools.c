#include <stdio.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlinear.h>
#include "fit_tools.h"

#define debug_print(fmt) \
        do { fprintf(stderr, "**ERROR**: at %s:%d:%s():\n" fmt,  __FILE__, \
                                __LINE__, __func__); } while (0)

void save_data_and_model(char *outfile, FinalPos *fp, Data *fit_data) {
    FILE *file = fopen(outfile, "w");
    if (file == NULL){
        fprintf(stderr, "Failed to open file '%s' for writing.%i, %s, %s",\
                outfile,  __LINE__, __FILE__, __func__);
        exit(-1);
    }

    for (int i = 0; i < DIM; ++i) {
        double ti = fit_data->t[i];
        double yi = fit_data->y[i];
        double fi = gaussian(fp->amp, fp->mu, fp->sig, ti);
        fprintf(file, "%.1f %.3f %.3f\n", ti, yi, fi);
    }
}

void save_smoothed_image(char *outfile, gsl_vector* data_sq[]) {
    FILE *f_img = fopen(outfile, "w");
    if (f_img == NULL) {
        fprintf(stderr, "Failed to open output file:%s. %i %s %s.\n", outfile,\
                __LINE__, __FILE__, __func__);
        exit(-2);
    }
    for (int i=0; i < DIM; i++) {
        for (int j=0; j < DIM; j++) {
            fprintf(f_img, "%d ", (uint16_t)gsl_vector_get(data_sq[i], j));
        }
        fprintf(f_img, "\n");
    }
    fclose(f_img);
}

void save_averaged_to_file(const char *outfile, gsl_vector *mux, gsl_vector *muy) {
    double x, y;
    FILE *f_sm = fopen(outfile, "w");
    if (f_sm == NULL){
        printf("Could not open file for writing:%s. %i %s %s.\n", outfile, \
                __LINE__, __FILE__, __func__);
        exit(-1);
    }
    for (int i=0; i < DIM; i++) {
        x = gsl_vector_get(mux, i);
        y = gsl_vector_get(muy, i);
        fprintf(f_sm, "%i %d %d\n", i, (uint16_t)x, (uint16_t)y);
    }
    fclose(f_sm);
}

void average_dim(gsl_vector *data_sq[], gsl_vector *mu_vec, const int axis) {
	double val, mu_new = 0, mu_old = 0;
    for (int i=0; i < DIM; i++){
		for (int j=0; j < DIM; j++){
            if (axis == 0)
                val = gsl_vector_get(data_sq[i], j);

            else
                val = gsl_vector_get(data_sq[j], i);

            if (j == 0)
                mu_old = mu_new = val;

            mu_new = mu_old + (val - mu_old) / (double)(j + 1);
            mu_old = mu_new;
		}
        gsl_vector_set(mu_vec, i, mu_new);
    }
}

void smooth_data_sq(gsl_vector *data_sq[], gsl_filter_gaussian_workspace* gauss_p) {
    // iterate over array smoothing
    for (int k=0; k < DIM; k++) {
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq[k], data_sq[k], gauss_p);
    }

    transpose_data_sq(data_sq);

    for (int k=0; k < DIM; k++) {
        gsl_filter_gaussian(GSL_FILTER_END_PADVALUE, ALPHA, 0, data_sq[k], data_sq[k], gauss_p);
    }
}

void transpose_data_sq(gsl_vector* data_sq[]) {
    for (int i=0; i < DIM; i++) {
        for (int j=0; j < DIM; j++) {
			double v = gsl_vector_get(data_sq[i], j);
            gsl_vector_set(data_sq[j], i, v);
        }
    }
}

void load_data_from_file(const char* infile_path, gsl_vector* data_sq[]) {
    int ret;
    FILE *infile;
    infile = fopen(infile_path, "rb");
    if (infile == NULL){
        debug_print();
        //printf("Could not open file:%s. %i %s:%s.\n", infile_path, __LINE__, __FILE__, __func__);
        exit(-1);
    }

    for (int i=0; i <= DIM; i++) {
        data_sq[i] = gsl_vector_alloc(DIM);
    }

    // Load data as ints from binary file.
    int temp, old_ret;
    for (int i=0; i < DIM; i++) {
        for(int j=0; j < DIM; j++){
            ret = fread(&temp, sizeof(uint16_t), 1, infile);
            old_ret = ret;
            if (ret < 0){
                fprintf(stderr, "fread failure at:%i. %i %s %s.\n", old_ret + 1, __LINE__, __FILE__, __func__);
                exit(1);
            }
            gsl_vector_set(data_sq[i], j, (double)temp);
        }
    }
}

double gaussian(const double a, const double b, const double c, const double t) {
	const double z = (t - b) / c;
	return (a * exp(-0.5 * z * z));
}

int func_f (const gsl_vector *x, void *params, gsl_vector * f) {
	Data *d = (Data*)params;
	double a = gsl_vector_get(x, 0);
	double b = gsl_vector_get(x, 1);
	double c = gsl_vector_get(x, 2);

	for (int i = 0; i < d->n; ++i) {
		double ti = d->t[i];
		double yi = d->y[i];
		double y = gaussian(a, b, c, ti);

		gsl_vector_set(f, i, yi - y);
	}

	return GSL_SUCCESS;
}

int func_df(const gsl_vector *x, void* params, gsl_matrix *J) {
    size_t n = ((Data*)params)->n;
    double *t = ((Data*)params)->t;
    double a = gsl_vector_get(x, 0); // Amp
    double b = gsl_vector_get(x, 1); // mu
    double c = gsl_vector_get(x, 2); // sig


    for (int i=0; i < n; i++) {
        double z = (t[i] - b) / c;
        double g = exp(-0.5 * z * z);

        gsl_matrix_set(J, i, 0, -g);
        gsl_matrix_set(J, i, 1, -(a / c) * g * z);
        gsl_matrix_set(J, i, 2, -(a / c) * g * z * z );
    }

    return GSL_SUCCESS;
}

void solve_system(FinalPos *fp, gsl_vector *x0, gsl_multifit_nlinear_fdf *fdf, gsl_multifit_nlinear_parameters *params, int axis) {
        const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
        const size_t max_iter = 30;
        const double xtol = 1.0e-8;
        const double gtol = 1.0e-8;
        const double ftol = 1.0e-8;
        const size_t n = fdf->n;
        const size_t p = fdf->p;
        gsl_multifit_nlinear_workspace *work = gsl_multifit_nlinear_alloc(T, params, n, p);
        gsl_vector *x = gsl_multifit_nlinear_position(work);
        int info;

        /* initialize solver */
        gsl_multifit_nlinear_init(x0, fdf, work);

        /* iterate until convergence */
        gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, NULL, NULL, &info, work);


        /* print summary */
        fprintf(stderr, "Axis %i NITER:%zu A:%.3f, Mu:%.3f, Sig:%.3f \n", \
            axis, \
            gsl_multifit_nlinear_niter(work),\
            gsl_vector_get(x, 0),\
            gsl_vector_get(x, 1),\
            gsl_vector_get(x, 2));

        fp->amp = gsl_vector_get(x, 0);
        fp->mu =  gsl_vector_get(x, 1);
        fp->sig = gsl_vector_get(x, 2);

        gsl_multifit_nlinear_free(work);
}

void rem_data_offset(Data *fit_data, int num) {
    double cur_min = fit_data->y[0];
    for (int i=1; i < num; i++){
        if (fit_data->y[i] < cur_min)
            cur_min = fit_data->y[i];
    }

    for (int j=0; j < num; j++) {
        fit_data->y[j] -= cur_min;
    }
}

int fit(gsl_vector *data_to_fit, FinalPos *fp, Data *fit_data, const int axis) {
    const size_t n = DIM;  /* number of data points to fit */
    const size_t p = 3;    /* number of model parameters */

    gsl_vector *f = gsl_vector_alloc(n);
    gsl_vector *x0 = gsl_vector_alloc(p);
    gsl_multifit_nlinear_fdf fdf;
    gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();


    /* load data */
    for (int i=0; i < n; ++i) {
        fit_data->t[i] = (double)i;
        fit_data->y[i] = gsl_vector_get(data_to_fit, i);
    }

    rem_data_offset(fit_data, n);

    /* define function to be minimized */
    fdf.f = func_f;
     fdf.df = NULL;  /* set to NULL for finite-difference Jacobian */
    //fdf.df = func_df;
    fdf.fvv = NULL; /* not using geodesic acceleration */
    fdf.n = n;
    fdf.p = p;
    fdf.params = fit_data;

    /* starting point */
    gsl_vector_set(x0, 0, 600.0); // amp
    gsl_vector_set(x0, 1, 130.0); // mu
    gsl_vector_set(x0, 2, 20.0);  // sig

    solve_system(fp, x0, &fdf, &fdf_params, axis);

    gsl_vector_free(f);
    gsl_vector_free(x0);

    return 0;
}
