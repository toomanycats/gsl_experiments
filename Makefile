CFLAGS += -Wall -std=c99
INCLUDE += `gsl-conifg --cflags`
LDFLAGS += `gsl-config  --libs`


all: data.bin smooth

data.bin: data.csv convert_ascii_data_to_binary.cpp
	g++ convert_ascii_data_to_binary.cpp -o convert_ascii_data_to_binary
	./convert_ascii_data_to_binary

smooth: smooth.c
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) $^ -o $@
