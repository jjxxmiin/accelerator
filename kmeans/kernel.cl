#include "kmeans.h"

__kernel void kmean( __global Point *data,
				__global Point *centroids,
				__global int* partitioned,
				int class_n,
				int data_n,
				float dbl_max)
{
	int i = get_global_id(0);
	
	Point t;
	int c_i;	

	float min_dist = DBL_MAX;
	
	for (c_i = 0; c_i < class_n; c_i++) {
		t.x = data[i].x - centroids[c_i].x;
		t.y = data[i].y - centroids[c_i].y;

		float dist = t.x * t.x + t.y * t.y;

		if (dist < min_dist) {
     			partitioned[i] = c_i;
			min_dist = dist;
		}
	}
}
