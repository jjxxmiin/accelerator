__kernel void kmeans(__global Point *data,
			__global Point *input_centroid,
			__global Point *output_centroid,
			__global int *partition,
			int class_n,int data_n,int iteration_n){
	
	int i = get_global_id(1);
	

}
