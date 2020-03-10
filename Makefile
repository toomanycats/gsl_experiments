CFLAGS += -Wall -std=c99 -g
INCLUDE += `gsl-config --cflags`
LDFLAGS += `gsl-config --libs`
AUX_FLAGS =

all: convert_ascii_to_binary data.bin fit_gaussian

convert_ascii_to_binary: convert_ascii_data_to_binary.cpp
	g++ $^ -o $@ $(AUX_FLAGS)

debug_convert:
	AUX_FLAGS = -D DEBUG=1
	convert_ascii_to_binary

fit_gaussian: fit_gaussian.c
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) $^ -o $@
	./fit_gaussian
	gnuplot plot_xy_means.txt

data.bin: convert_ascii_data_to_binary.cpp
	./convert_ascii_to_binary

plot_orig:
	xargs -n 299 < data.csv > orig.ascii
	gnuplot -p plot_orig.txt

clean:
	rm data.bin convert_ascii_to_binary fit_gaussian orig.ascii data_sm.txt smoothed_image.txt

.PHONY: clean plot_orig
