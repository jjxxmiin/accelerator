#include <CL/cl.h>
#include "kmeans.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
/*
 * TODO
 * Define global variables here. For example,
 * cl_platform_id platform;
 */
#define CHECK_ERROR(err) \
	if(err != CL_SUCCESS){ \
		printf("[%s,%d] OpenCL error %d\n",__FILE__,__LINE__,err); \
		exit(EXIT_FAILURE); \
	}

cl_platform_id platform;
cl_device_id device;
cl_context context;
//cl_command_queue queue;
cl_command_queue queue[2];
cl_program program;
char *kernel_source;
size_t kernel_source_size;
cl_kernel kernel;
cl_int err;
cl_event kernel_event[2];

double get_time() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (double)tv.tv_sec + (double)1e-6 * tv.tv_usec;
}

char *get_source_code(const char *file_name,size_t *len){
	char *source_code;
	size_t length;
	FILE *file = fopen(file_name,"r");
	if(file == NULL){
		printf("[%s,%d] OpenCL error %s\n",__FILE__,__LINE__,file_name);
		exit(EXIT_FAILURE); 
	}
	fseek(file,0,SEEK_END);
	length = (size_t)ftell(file);
	rewind(file);

	source_code = (char *)malloc(length + 1);
	fread(source_code,length,1,file);
	source_code[length] = '\0';

	fclose(file);

	*len = length;
	return source_code;
}

void kmeans_init() {
	err = clGetPlatformIDs(1,&platform,NULL);
	CHECK_ERROR(err);

	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_GPU,1,&device,NULL);
	CHECK_ERROR(err);

	context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
	CHECK_ERROR(err);

	queue[0] = clCreateCommandQueue(context,device,0,&err);
	CHECK_ERROR(err);
	queue[1] = clCreateCommandQueue(context,device,0,&err);
	CHECK_ERROR(err);	

	kernel_source = get_source_code("kernel.cl", &kernel_source_size);
	program = clCreateProgramWithSource(context,1,(const char**)&kernel_source,&kernel_source_size,&err);
	CHECK_ERROR(err);

	err = clBuildProgram(program,1,&device,"",NULL,NULL);
	if(err == CL_BUILD_PROGRAM_FAILURE){
		size_t log_size;
		char *log;

		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,0,NULL,&log_size);
		CHECK_ERROR(err);

		log = (char*)malloc(log_size+1);
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG, log_size, log,NULL);
		CHECK_ERROR(err);

		log[log_size] = '\0';
		printf("Compiler error:\n%s\n",log);
		free(log);
	}
	CHECK_ERROR(err);

	kernel = clCreateKernel(program, "kmean",&err);
	CHECK_ERROR(err);	
}

void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* partitioned)
{
	float dbl_max = DBL_MAX;

	cl_mem buf_data, buf_cen, buf_part;
	//data
	buf_data = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Point)*data_n, NULL, &err);
	CHECK_ERROR(err);
	//input_centroid
	buf_cen = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(Point)*class_n, NULL, &err);
	CHECK_ERROR(err);
	//partition
	buf_part = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int)*data_n, NULL, &err);
	CHECK_ERROR(err);
	
	//Write data,partition
	err = clEnqueueWriteBuffer(queue[0], buf_data, CL_FALSE, 0, sizeof(Point)*data_n, data, 0, NULL, NULL);
	CHECK_ERROR(err);

	err = clEnqueueWriteBuffer(queue[0], buf_part,CL_FALSE,0,sizeof(int)*data_n,partitioned,0,NULL, NULL);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf_data);
	CHECK_ERROR(err);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf_cen);
	CHECK_ERROR(err);
	err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buf_part);
	CHECK_ERROR(err);
	err = clSetKernelArg(kernel, 3, sizeof(int), &class_n);
	CHECK_ERROR(err);
	err = clSetKernelArg(kernel, 4, sizeof(int), &data_n);
	CHECK_ERROR(err);
	err = clSetKernelArg(kernel, 5, sizeof(float), &dbl_max);
	CHECK_ERROR(err);	
	
	size_t global_size = data_n;
        size_t local_size = 256;
	global_size = (global_size + local_size - 1) / local_size * local_size;

	int i,class_i,data_i;
	
	for(i=0;i<iteration_n;i++) {
		//Write centroid
		err = clEnqueueWriteBuffer(queue[1],buf_cen,CL_TRUE,0,sizeof(Point)*class_n,centroids,0,NULL,&kernel_event[1]);
                CHECK_ERROR(err);

		//Kernel open
		err = clEnqueueNDRangeKernel(queue[0],kernel,1,NULL,&global_size,&local_size,0,NULL,&kernel_event[0]);
		CHECK_ERROR(err);
		
		err = clFinish(queue[0]);
		CHECK_ERROR(err);

		//Read partition
		err = clEnqueueReadBuffer(queue[1],buf_part,CL_TRUE,0,sizeof(int)*data_n,partitioned,0,NULL,&kernel_event[0]);
                CHECK_ERROR(err);		
		
		int* count = (int*)malloc(sizeof(int) * class_n);

		for (class_i = 0; class_i < class_n; class_i++) {
			centroids[class_i].x = 0.0;
			centroids[class_i].y = 0.0;
			count[class_i] = 0;
		}

		for (data_i = 0; data_i < data_n; data_i++) {         
			centroids[partitioned[data_i]].x += data[data_i].x;
			centroids[partitioned[data_i]].y += data[data_i].y;
			count[partitioned[data_i]]++;
		}

		for (class_i = 0; class_i < class_n; class_i++) {
			centroids[class_i].x /= count[class_i];
			centroids[class_i].y /= count[class_i];
		}
	}
	clReleaseMemObject(buf_data);
	clReleaseMemObject(buf_cen);
	clReleaseMemObject(buf_part);
	clReleaseContext(context);
	clReleaseCommandQueue(queue[0]);
	clReleaseCommandQueue(queue[1]);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
}
