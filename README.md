# Practice using GSL for Gaussin Fitting
At the ALS we use Prosilica cameras to watch the beam profile when tunning for
injection as well as diagnostics. The physics group is experimenting with ML tools
to correct feed forward non-linear effects and uses the parameters of Gaussian fitting.

In production we use EPICS module AreaDetector which has various plugins or callbacks
written in C++ that perform analysis on the image. This C program will eventually
become such a plugin if it proves useful.

## Input Test Data
Ascii data `data.csv` comes from `caget channel_name`.
The CPP program `create_binary_data.cpp` converts the ascii data into a binary file
`data.bin`.

## GSL
GSL is then used to read the binary data and  perform the following steps:

1. 2D Gaussian smoothing on 299 x 299 image.
	1. kernel size = 51
	1. alpha = 10.0
1. Aggregate cols
	1. Using the mean of the axes 0 and then 1
1. Fit Gaussian curve to the `mean_x` and `mean_y`

Smoothing is optional and appears to not help the fit time or quality.

### GSL Example Pages
* https://www.gnu.org/software/gsl/doc/html/filter.html
* https://www.gnu.org/software/gsl/doc/html/nls.html

# Viewing the Beam Profile
## Original Beam Profile
![Original Beam Profile](orig_beam_profile.png)

## Smoothed Beam Profile
To get a good fast fit, I propose firt smoothing the 2D beam profile image
with gaussian kernel.

You can see from the plot, that there's a Y offset that can be removed before
fitting. The conversion from pixels to microns is ~1.7 um/pixel.

![Smoothed Beam](smoothed_beam_profile.png)

## Fit Params
The fit is performed on the two single dimension reductions (average) of the
2D image.

This plot are the two averaged rows and averaged columns:
![Smoothed Averages](smooth_ave_plot.png)


This plot is the Gaussian fit one of the above:
![Gaussian Fit](gaussian_fit.png)
