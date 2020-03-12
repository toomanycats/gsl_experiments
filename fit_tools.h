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
