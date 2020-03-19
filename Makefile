CFLAGS += -Wall -std=c99
INCLUDE += `gsl-config --cflags`
LDFLAGS += `gsl-config --libs`
# for camonitor version
INC =  -I/usr/local/epics/R3.15/base-3.15/include
INC += -I/usr/local/epics/R3.15/base-3.15/include/os/Linux
INC += -I/usr/local/epics/R3.15/base-3.15/src/ca/client/tools
INC += -I/usr/local/epics/R3.15/base-3.15/include/compiler/gcc
LIB += -L/usr/local/epics/R3.15/base/lib/linux-x86_64
TOOLS = /usr/local/epics/R3.15.4/base-3.15/src/ca/client/tools/O.linux-x86_64/tool_lib.o

all: convert_ascii_to_binary data.bin fit_tools.o fit_gaussian data_model_%.ascii

fit_gaussian_camonitor: fit_gaussian_camonitor.c
	gcc $(CFLAGS) -g  $(INCLUDE) $(LDFLAGS) $(INC) $(LIB) -lca -lCom $(TOOLS) fit_tools.o  $^ -o $@

get_data:
	export EPICS_CA_MAX_ARRAY_BYTES=10000000
	caget BL31:image1:ArrayData | tr " " "\n" | head -n 89401 > data.csv

convert_ascii_to_binary: convert_ascii_data_to_binary.cpp
	g++ $^ -o $@ $(AUX_FLAGS)

fit_tools.o: fit_tools.c fit_tools.h
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) $^ -c fit_tools.c

fit_gaussian: fit_gaussian.c fit_tools.o
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) fit_tools.o fit_gaussian.c -o fit_gaussian

data_model_%.ascii: plot_gaussian_fit.txt
	./fit_gaussian
	gnuplot plot_gaussian_fit.txt

data.bin: convert_ascii_data_to_binary.cpp
	./convert_ascii_to_binary

clean:
	rm data.bin convert_ascii_to_binary fit_gaussian
	test -f fit_tools.o && rm fit_tools.o
	test -f orig_beam_profile_img.ascii && rm orig_beam_profile_img.ascii
	test -f rows_cols_ave.ascii && rm rows_cols_ave.ascii
	test -f smoothed_image.ascii && rm smoothed_image.ascii
	test -f data_model_x.ascii && rm data_model_x.ascii
	test -f data_model_y.ascii && rm data_model_y.ascii

.PHONY: clean plot_orig get_data
