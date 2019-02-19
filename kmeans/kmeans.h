#ifndef __KMENAS_H__
#define __KMEANS_H__

typedef struct{
    float x, y;
} Point;

void kmeans_init();
void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* clsfy_result);

#endif // __KMEANS_H__

