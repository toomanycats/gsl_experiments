CFLAGS += -Wall -std=c99 -g3
INCLUDE += `gsl-config --cflags`
LDFLAGS += `gsl-config --libs`
AUX_FLAGS =

all: convert_ascii_to_binary data.bin smooth

convert_ascii_to_binary: convert_ascii_data_to_binary.cpp
	g++ $^ -o $@ $(AUX_FLAGS)

debug_convert: AUX_FLAGS = -D DEBUG=1
debug_convert: convert_ascii_to_binary
debug_convert: data.bin

smooth: smooth.c
	gcc $(CFLAGS) $(INCLUDE) $(LDFLAGS) $^ -o $@

data.bin: convert_ascii_data_to_binary.cpp
	./convert_ascii_to_binary

clean:
	rm data.bin convert_ascii_to_binary smooth

.PHONY: clean debug_convert
