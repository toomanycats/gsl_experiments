#include <stdio.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>

struct data
{
  double *t;
  double *y;
  size_t n;
};

/* model function: a * exp( -1/2 * [ (t - b) / c ]^2 ) */
double gaussian(const double a, const double b, const double c, const double t) {
	const double z = (t - b) / c;
	return (a * exp(-0.5 * z * z));
}

int func_f (const gsl_vector * x, void *params, gsl_vector * f) {
	struct data *d = (struct data *) params;
	double a = gsl_vector_get(x, 0);
	double b = gsl_vector_get(x, 1);
	double c = gsl_vector_get(x, 2);
	size_t i;

	for (i = 0; i < d->n; ++i)
	{
		double ti = d->t[i];
		double yi = d->y[i];
		double y = gaussian(a, b, c, ti);

		gsl_vector_set(f, i, yi - y);
	}

	return GSL_SUCCESS;
}

void solve_system(gsl_vector *x, gsl_multifit_nlinear_fdf *fdf, gsl_multifit_nlinear_parameters *params) {
  const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
  const size_t max_iter = 200;
  const double xtol = 1.0e-8;
  const double gtol = 1.0e-8;
  const double ftol = 1.0e-8;
  const size_t n = fdf->n;
  const size_t p = fdf->p;
  gsl_multifit_nlinear_workspace *work = gsl_multifit_nlinear_alloc(T, params, n, p);
  gsl_vector * f = gsl_multifit_nlinear_residual(work);
  gsl_vector * y = gsl_multifit_nlinear_position(work);
  int info;

  /* initialize solver */
  gsl_multifit_nlinear_init(x, fdf, work);

  /* iterate until convergence */
  gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, NULL, NULL, &info, work);


  /* print summary */
  fprintf(stderr, "NITER         = %zu\n", gsl_multifit_nlinear_niter(work));
  fprintf(stderr, "final x       = (%.12e, %.12e, %12e)\n", gsl_vector_get(x, 0), gsl_vector_get(x, 1), gsl_vector_get(x, 2));

  gsl_multifit_nlinear_free(work);
}

int main (void) {
  const size_t n = 299;  /* number of data points to fit */
  const size_t p = 3;    /* number of model parameters */
  const double a = 800.0;  /* amplitude */
  const double b = 150.0;  /* center */
  const double c = 43.0; /* width */
  gsl_vector *f = gsl_vector_alloc(n);
  gsl_vector *x = gsl_vector_alloc(p);
  gsl_multifit_nlinear_fdf fdf;
  gsl_multifit_nlinear_parameters fdf_params = gsl_multifit_nlinear_default_parameters();
  struct data fit_data;

  fit_data.t = malloc(n * sizeof(double));
  fit_data.y = malloc(n * sizeof(double));
  fit_data.n = n;

  int ret;
  double ind, y1, y2;
  /* load smoothed data */
	FILE *infile = fopen("data_sm.txt", "r");
	for (int i=0; i < n; ++i) {
		double t = (double)i / (double) n;
		//read csv smoothed data
		ret = fscanf(infile, "%i %d %d", ind, y1, y2);
        printf("fscanf ret:%i\n", ret);
		fit_data.t[i] = t;
		fit_data.y[i] = y1;
	}

  /* define function to be minimized */
  fdf.f = func_f;
  fdf.n = n;
  fdf.p = p;
  fdf.params = &fit_data;

  /* starting point */
  gsl_vector_set(x, 0, 400.0);
  gsl_vector_set(x, 1, 145.0);
  gsl_vector_set(x, 2, 35.0);

  solve_system(x, &fdf, &fdf_params);

  /* print data and model */
  {
    double A = gsl_vector_get(x, 0);
    double B = gsl_vector_get(x, 1);
    double C = gsl_vector_get(x, 2);

    for (int i = 0; i < n; ++i)
      {
        double ti = fit_data.t[i];
        double yi = fit_data.y[i];
        double fi = gaussian(A, B, C, ti);

        printf("%f %f %f\n", ti, yi, fi);
      }
  }

  gsl_vector_free(f);
  gsl_vector_free(x);

  return 0;
}
