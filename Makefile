CFLAGS += -Wall -std=c99
INCLUDE += `gsl-config --cflags`
LDFLAGS += `gsl-config --libs`

all: plot_orig convert_ascii_to_binary data.bin fit_gaussian plot_fit

convert_ascii_to_binary: convert_ascii_data_to_binary.cpp
	g++ $^ -o $@ $(AUX_FLAGS)

fit_tools.o: fit_tools.c fit_tools.h
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) $^ -c fit_tools.c

fit_gaussian: fit_gaussian.c fit_tools.o
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) fit_tools.o fit_gaussian.c -o fit_gaussian

plot_fit: fit_gaussian
	./fit_gaussian False
	gnuplot plot_gaussian_fit.txt

data.bin: convert_ascii_data_to_binary.cpp
	./convert_ascii_to_binary

plot_orig:
	xargs -n 299 < data.csv > orig_beam_profile_img.txt
	gnuplot -p plot_orig.txt

clean:
	rm fit_tools.o *.png data.bin convert_ascii_to_binary fit_gaussian
	rm orig_beam_profile_img.txt data_sm.txt smoothed_image.txt

.PHONY: clean plot_orig plot_fit
