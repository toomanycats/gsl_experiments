CFLAGS += -Wall -std=c99 -lgslcblas
LDFLAGS += -lgsl

all: smooth

smooth: smooth.c
	gcc $(CFLAGS) $(LDFLAGS) $^ -o $@
