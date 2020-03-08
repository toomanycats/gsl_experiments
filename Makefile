CFLAGS += -Wall -std=c99 -g3
INCLUDE += `gsl-config --cflags`
LDFLAGS += `gsl-config --libs`
AUX_FLAGS =

all: convert_ascii_to_binary data.bin smooth

convert_ascii_to_binary: convert_ascii_data_to_binary.cpp
	g++ $^ -o $@ $(AUX_FLAGS)

debug_convert:
	AUX_FLAGS = -D DEBUG=1
	convert_ascii_to_binary

smooth: smooth.c
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) $^ -o $@

data.bin: convert_ascii_data_to_binary.cpp
	./convert_ascii_to_binary

plot_orig:
	xargs -n 299 < data.csv > orig.ascii
	gnuplot -p plot_orig.txt

clean:
	rm data.bin convert_ascii_to_binary smooth orig.ascii

.PHONY: clean debug_convert plot_orig
