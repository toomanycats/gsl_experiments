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
	./fit_gaussian
	gnuplot plot_gaussian_fit.txt

data.bin: convert_ascii_data_to_binary.cpp
	./convert_ascii_to_binary

plot_orig:
	xargs -n 299 < data.csv > orig_beam_profile_img.txt
	gnuplot -p plot_orig.txt

clean:
	rm *.png data.bin convert_ascii_to_binary fit_gaussian
	test -f fit_tools.o && rm fit_tools.o
	test -f orig_beam_profile_img.txt && rm orig_beam_profile_img.txt
	test -f data_sm.txt && rm data_sm.txt
	test -f smoothed_image.txt && rm smoothed_image.txt
	test -f data_model_x.txt && rm data_model_x.txt
	test -f data_model_y.txt && rm data_model_x data_model_y

.PHONY: clean plot_orig plot_fit
