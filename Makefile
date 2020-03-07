CFLAGS += -Wall -std=c99 
LDFLAGS += -lgsl -lgslcblas

all: data.bin smooth

data.bin: data.csv
	g++ create_binary_data.cpp -o convert_ascii_data_to_binary
	./convert_ascii_data_to_binary

smooth: smooth.c
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@
