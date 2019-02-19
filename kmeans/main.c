/*
  Sequential implementation of KMeans
*/

#include "kmeans.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_DIM 2
#define DEFAULT_ITERATION 1024

#define GET_TIME(T) __asm__ __volatile__ ("rdtsc\n" : "=A" (T))


// Read data from file
void read_data(FILE* f, int * class_n, int * data_n, float ** centroid_p, float** data_p);
int timespec_subtract(struct timespec*, struct timespec*, struct timespec*);


int main(int argc, char** argv)
{
    int class_n, data_n, iteration_n;
    float *centroids, *data;
    int* partitioned;
    FILE *io_file;
    struct timespec start, end, spent;

    // Check parameters
    if (argc < 4) {
        fprintf(stderr, "usage: %s <input file> <output file> <Maximum # of iteration>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Read initial centroid data
    io_file = fopen(argv[1], "rb");
    if (io_file == NULL) {
        fprintf(stderr, "File open error %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    read_data(io_file, &class_n, &data_n, &centroids, &data);
    fclose(io_file);

    iteration_n = atoi(argv[3]);
        
    partitioned = (int*)malloc(sizeof(int)*data_n);

    kmeans_init();

    clock_gettime(CLOCK_MONOTONIC, &start);
    // Run Kmeans algorithm
    kmeans(iteration_n, class_n, data_n, (Point*)centroids, (Point*)data, partitioned);
    clock_gettime(CLOCK_MONOTONIC, &end);

    timespec_subtract(&spent, &end, &start);
    printf("Time spent: %ld.%09ld\n", spent.tv_sec, spent.tv_nsec);

    // Write output
    io_file = fopen(argv[2], "wb");
    fwrite(&class_n, sizeof(class_n), 1, io_file);
    fwrite(&data_n, sizeof(data_n), 1, io_file);
    fwrite(centroids, sizeof(Point), class_n, io_file); 
    fwrite(partitioned, sizeof(int), data_n, io_file); 
    fclose(io_file);

    // Free allocated buffers
    free(centroids);
    free(data);
    free(partitioned);
    return 0;
}


     
int timespec_subtract (struct timespec* result, struct timespec *x, struct timespec *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_nsec < y->tv_nsec) {
        int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
        y->tv_nsec -= 1000000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_nsec - y->tv_nsec > 1000000000) {
        int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
        y->tv_nsec += 1000000000 * nsec;
        y->tv_sec -= nsec;
    }
     
    /* Compute the time remaining to wait.
       tv_nsec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_nsec = x->tv_nsec - y->tv_nsec;
    
    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}


void read_data(FILE* f, int * class_n, int * data_n, float ** centroid_p, float** data_p)
{
    size_t r;

    r = fread(class_n, sizeof(int), 1, f);
    if (r < 1) {
        fputs("Error reading file size", stderr);
        exit(EXIT_FAILURE);
    }
    r = fread(data_n, sizeof(int), 1, f);
    if (r < 1) {
        fputs("Error reading file size", stderr);
        exit(EXIT_FAILURE);
    }
    
    *centroid_p = (float*)malloc(sizeof(float) * DATA_DIM * (*class_n));
    *data_p = (float*)malloc(sizeof(float) * DATA_DIM * (*data_n));

    r = fread(*centroid_p, sizeof(float), DATA_DIM * (*class_n), f);
    if (r < DATA_DIM * (*class_n)) {
        fputs("Error reading data", stderr);
        exit(EXIT_FAILURE);
    }
    r = fread(*data_p, sizeof(float), DATA_DIM * (*data_n), f);
    if (r < DATA_DIM * (*data_n)) {
        fputs("Error reading data", stderr);
        exit(EXIT_FAILURE);
    }
}

