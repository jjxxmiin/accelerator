#include <CL/cl.h>
#include "kmeans.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
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
cl_command_queue queue;
cl_program program;
char *kernel_source;
size_t kernel_source_size;
cl_kernel kernel;
cl_int err;

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
	/*
	 * TODO
	 * Initialize OpenCL objects as global variables. For example,
	 * clGetPlatformIDs(1, &platform, NULL);
	 */
	err = clGetPlatformIDs(1,&platform,NULL);
	CHECK_ERROR(err);

	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_GPU,1,&device,NULL);
	CHECK_ERROR(err);

	context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
	CHECK_ERROR(err);

	queue = clCreateCommandQueue(context,device,0,&err);
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
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,0,NULL,&log_size);
		CHECK_ERROR(err);

		log[log_size] = '\0';
		printf("Compiler error:\n%s\n",log);
		free(log);
	}
	CHECK_ERROR(err);
	
	kernel = clCreateKernel(program, "kmeans",&err);
	CHECK_ERROR(err);	
}

void kmeans(int iteration_n, int class_n, int data_n, Point* centroids, Point* data, int* partitioned)
{
	/*
	 * TODO
	 * Implement here.
	 * See "kmeans_seq.c" if you don't know what to do.
	 */
	kmeans_init();
	cl_mem bufA,bufB,bufC,bufD;
	// bufA = data
	bufA = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(Point)*data_n,NULL,&err);
	CHECK_ERROR(err);
	// bufB = centroid
	bufB = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(Point)*class_n,NULL,&err);
        CHECK_ERROR(err);
	// bufC = centroid
	bufC = clCreateBuffer(context,CL_MEM_WRITE_ONLY,sizeof(Point)*class_n,NULL,&err);
	CHECK_ERROR(err);
	// bufD = partition
	bufD = clCreateBuffer(context,CL_MEM_WRITE_ONLY,sizeof(int)*dara_n,NULL,&err);
	CHECK_ERROR(err);
	
	double start_time = get_time();

	err = clEnqueueWriteBuffer(queue,bufA,CL_FALSE,0,sizeof(Point)*data_n,data,0,NULL,NULL);
	CHECK_ERROR(err);

	err = clEnqueueWriteBuffer(queue,bufB,CL_FALSE,0,sizeof(Point)*class_n,centroids,0,NULL,NULL);
	CHECK_ERROR(err);
	
	err = clSetKernelArg(kernel,0,sizeof(cl_mem),&bufA);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,1,sizeof(cl_mem),&bufB);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,2,sizeof(cl_mem),&bufC);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,3,sizeof(cl_mem),&bufD);
	CHECK_ERROR(err);
	
	err = clSetKernelArg(kernel,4,sizeof(cl_int),&class_n);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,5,sizeof(cl_int),&data_n);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,6,sizeof(cl_int),&iteration_n);	
	CHECK_ERROR(err);

	size_t global_size = data_n*interation_n*class_n;
	size_t local_size = 256;

	err = clEnqueueNDRangeKernel(queue,kernel,1,NULL,global_size,local_size,0,NULL,NULL);
	CHECK_ERROR(err);
	
	err = clEnqueueReadBuffer(queue,bufC,CL_TRUE,0,sizeof(Point)*class_n,centroids,0,NULL,NULL);
	CHECK_ERROR(err);

	err = clEnqueueReadBuffer(queue,bufD,CL_TRUE,0,sizeof(int)*data_n,partitioned,0,NULL,NULL);
	CHECK_ERROR(err);

	double end_time = get_time();
	printf("elapsed time : %f\n",end_time-start_time);

	clReleaseMemObject(bufA);
	clReleaseMemObject(bufB);
	clReleaseMemObject(bufC);
        clReleaseMemObject(bufD);
	clReleaseKernel(kernel);
        clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}
