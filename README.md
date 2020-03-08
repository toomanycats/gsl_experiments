# Practice using GSL for Gaussin Fitting
There's a Makefile which will create the binary from the ascii file `data.csv`
and then runs the GSL steps.

## Input Test Data
Ascii data `data.csv` comes from `caget channel_name`.
The CPP program `create_binary_data.cpp` converts the ascii data into a binary file
`data.bin`.

## GSL
GSL is then used to read the binary data and first
    1. 2D Gaussian smoothing
    1. Aggregate cols and rows to a `mean_x` and `mean_y`
    1. Fit Gaussian curve to the `mean_x` and `mean_y`

### GSL Example Page
https://www.gnu.org/software/gsl/doc/html/filter.html

# Viewing the Beam Profile
## Original Beam Profile
![Original Beam Profile](orig_beam_profile.png)

## Smoothed Beam Profile with Fit
TODO

# Sanity Checking Binary Conversion
	tail -n 5 data.csv

	21
	61
	165
	91
	0

	hexdump data.bin | tail -n 5

	00574b0 0031 0000 0024 0000 0000 0000 0011 0000
	00574c0 0039 0000 0014 0000 0048 0000 0010 0000
	00574d0 0015 0000 003d 0000 00a5 0000 005b 0000
	00574e0 0000 0000                              
	00574e4

