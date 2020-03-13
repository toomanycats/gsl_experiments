#include <stdint.h>
#include <gsl/gsl_filter.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlinear.h>

#define DIM 299 /* square image */
#define ALPHA 10.0
#define K_SIZE 51 /* window size */
#define INFILE "data.bin"

typedef struct FinalPos {
    double amp;
    double mu;
    double sig;
} FinalPos;

typedef struct Data {
    double *t;
    double *y;
    size_t n;
} Data;

void save_data_and_model(char *outfile, FinalPos *fp, Data *fit_data);
void save_smoothed_image(char *outfile, gsl_vector*[]);
void save_averaged_to_file(const char *outfile, gsl_vector *mux, gsl_vector *muy);
void average_dim(gsl_vector *data_sq[], gsl_vector *mux, int);
void smooth_data_sq(gsl_vector *data_sq[], gsl_filter_gaussian_workspace* gauss_p);
void transpose_data_sq(gsl_vector* dat_sq[]);
void load_data_from_file(gsl_vector* data_sq[]);
double gaussian(const double a, const double b, const double c, const double t);
void solve_system(FinalPos *fp, gsl_vector *x0, gsl_multifit_nlinear_fdf *fdf, gsl_multifit_nlinear_parameters *params);
void rem_data_offset(Data *fit_data, int num);
int fit(gsl_vector *data_to_fit, FinalPos *fp, Data *fit_data);
