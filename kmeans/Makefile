
CC=gcc
CFLAGS=-Wall


LIBS = -lrt
LDFLAGS = ${LIBS}


all: seq opencl

.PHONY: all seq opencl clean


seq: kmeans_seq

kmeans_seq: kmeans_seq.o main.o
	${CC} $^ -o $@ ${LDFLAGS}


opencl: kmeans_opencl

kmeans_opencl: kmeans_opencl.o main.o
	${CC} $^ -o $@ ${LDFLAGS} -lOpenCL

clean:
	rm -f kmeans kmeans_opencl kmeans_seq main.o kmeans_seq.o kmeans_opencl.o
