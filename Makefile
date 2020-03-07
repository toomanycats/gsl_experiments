CFLAGS += -Wall -std=c99 -lgslcblas
LDFLAGS += -lgsl

all: smooth data.bin

data.bin: data.csv
	g++ create_binary_data.cpp -o convert_ascii_data_to_binary
	./convert_ascii_data_to_binary

smooth: smooth.c
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@
