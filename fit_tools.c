#include <stdio.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlinear.h>

void save_smoothed_image(char *outfile, gsl_vector* data_sq[]) {
    FILE *f_img = fopen(outfile, "w");
    if (f_img == NULL) {
        fprintf(stderr, "Failed to open output file for smoothed image.\n");
        exit(-2);
    }
    for (int i=0; i < DIM; i++) {
        for (int j=0; j < DIM; j++) {
            fprintf(f_img, "%f ", gsl_vector_get(data_sq[i], j));
        }
        fprintf(f_img, "\n");
    }
    fclose(f_img);
}

void save_averaged_to_file(const char *outfile, gsl_vector *mux, gsl_vector *muy) {
    double x, y;
    FILE *f_sm = fopen(outfile, "w");
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
}

void average_dim(gsl_vector *data_sq[], gsl_vector *mux){
	double x, mu_x, mu_old_x;
    for (int i=0; i < DIM; i++){
		for (int j=0; j < DIM; j++){
			x = gsl_vector_get(data_sq[i], j);
            if (j == 0) {
                mu_old_x = x;
            }
            mu_x = mu_old_x + (x - mu_old_x) / (double)(j + 1);
            mu_old_x = mu_x;
		}
        gsl_vector_set(mux, i, mu_x);
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

void load_data_from_file(gsl_vector* data_sq[]) {
    int ret;
    FILE *infile;
    infile = fopen(INFILE, "rb");
    if (infile == NULL){
        printf("did not open file.\n");
        exit(-1);
    }

    for (int i=0; i <= DIM; i++) {
        data_sq[i] = gsl_vector_alloc(DIM);
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
}

double gaussian(const double a, const double b, const double c, const double t) {
	const double z = (t - b) / c;
	return (a * exp(-0.5 * z * z));
}

int func_f (const gsl_vector *x, void *params, gsl_vector * f) {
	struct data *d = (struct data *) params;
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

void solve_system(struct final_pos *fp, gsl_vector *x0, gsl_multifit_nlinear_fdf *fdf, gsl_multifit_nlinear_parameters *params) {
        const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
        const size_t max_iter = 200;
        const double xtol = 1.0e-8;
        const double gtol = 1.0e-8;
        const double ftol = 1.0e-8;
        const size_t n = fdf->n;
        const size_t p = fdf->p;
        gsl_multifit_nlinear_workspace *work = gsl_multifit_nlinear_alloc(T, params, n, p);
        //gsl_vector *f = gsl_multifit_nlinear_residual(work);
        gsl_vector *x = gsl_multifit_nlinear_position(work);
        int info;

        /* initialize solver */
        gsl_multifit_nlinear_init(x0, fdf, work);

        /* iterate until convergence */
        gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, NULL, NULL, &info, work);


        /* print summary */
        fprintf(stderr, "NITER         = %zu\n", gsl_multifit_nlinear_niter(work));
        fprintf(stderr, "final x       = (%.12e, %.12e, %12e)\n", gsl_vector_get(x, 0), gsl_vector_get(x, 1), gsl_vector_get(x, 2));

        fp->amp = gsl_vector_get(x, 0);
        fp->mu =  gsl_vector_get(x, 1);
        fp->sig = gsl_vector_get(x, 2);

        gsl_multifit_nlinear_free(work);
}

void rem_data_offset(struct data *fit_data, int num) {
    double cur_min = fit_data->y[0];
    for (int i=1; i < num; i++){
        if (fit_data->y[i] < cur_min)
            cur_min = fit_data->y[i];
    }

   for (int j=0; j < num; j++){
      fit_data->y[j] -= cur_min;
   }
}

int fit(gsl_vector *data_to_fit) {
    const size_t n = DIM;  /* number of data points to fit */
    const size_t p = 3;    /* number of model parameters */

    struct final_pos fp;
    gsl_vector *f = gsl_vector_alloc(n);
    gsl_vector *x0 = gsl_vector_alloc(p);
    gsl_multifit_nlinear_fdf fdf;
    gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();

    struct data fit_data;
    fit_data.t = malloc(n * sizeof(double));
    fit_data.y = malloc(n * sizeof(double));
    fit_data.n = n;

    /* load smoothed data */
    for (int i=0; i < n; ++i) {
        fit_data.t[i] = (double)i;
        fit_data.y[i] = gsl_vector_get(data_to_fit, i);
    }

    rem_data_offset(&fit_data, n);

    /* define function to be minimized */
    fdf.f = func_f;
    fdf.fvv = NULL; /* not using geodesic acceleration */
    fdf.df = NULL;  /* set to NULL for finite-difference Jacobian */
    fdf.n = n;
    fdf.p = p;
    fdf.params = &fit_data;

    /* starting point */
    gsl_vector_set(x0, 0, 700.0); // amp
    gsl_vector_set(x0, 1, 140.0); // mu
    gsl_vector_set(x0, 2, 20.0);  // sig

    solve_system(&fp, x0, &fdf, &fdf_params);

    /* print data and model */
    printf("index data fit\n");
    for (int i = 0; i < n; ++i) {
        double ti = fit_data.t[i];
        double yi = fit_data.y[i];
        double fi = gaussian(fp.amp, fp.mu, fp.sig, ti);

        printf("%.1f %.3f %.3f\n", ti, yi, fi);
    }

    gsl_vector_free(f);
    gsl_vector_free(x0);

    return 0;
}
