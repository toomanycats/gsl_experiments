#include <stdint.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlinear.h>

#define DIM 299 /* square image */
#define ALPHA 10.0
#define K_SIZE 51 /* window size */
#define INFILE "data.bin"

struct final_pos {
    double amp;
    double mu;
    double sig;
};

struct data {
    double *t;
    double *y;
    size_t n;
};

int fit(gsl_vector*);

void save_smoothed_image(char *outfile, gsl_vector*[]);
void save_averaged_to_file(const char *outfile, gsl_vector *mux, gsl_vector *muy);
void average_dim(gsl_vector *data_sq[], gsl_vector *mux);
void smooth_data_sq(gsl_vector *data_sq[], gsl_filter_gaussian_workspace* gauss_p);
void transpose_data_sq(gsl_vector* dat_sq[]);
void load_data_from_file(gsl_vector* data_sq[]);
double gaussian(const double a, const double b, const double c, const double t);
void solve_system(struct final_pos *fp, gsl_vector *x0, gsl_multifit_nlinear_fdf *fdf, gsl_multifit_nlinear_parameters *params);
void rem_data_offset(struct data *fit_data, int num);
int fit(gsl_vector *data_to_fit);
