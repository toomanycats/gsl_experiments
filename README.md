# Practice using GSL for Gaussin Fitting

## Input Test Data
Ascii data `data.csv` comes from `caget channel_name`.
The CPP program `create_binary_data.cpp` converts the ascii data into a binary file
`data.bin`.

## GSL
GSL is then used to read the binary data and first
    1. 2D Gaussian smoothing
    1. Aggregate cols and rows to a `mean_x` and `mean_y`
    1. Fit Gaussian curve to the `mean_x` and `mean_y`



