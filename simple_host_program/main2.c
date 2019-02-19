#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <CL/cl.h>

#define CHECK_ERROR(err) \
	if (err != CL_SUCCESS) { \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
		exit(EXIT_FAILURE); \
	}

char *get_source_code(const char *file_name, size_t *len) {
	char *source_code;
	size_t length;
	FILE *file = fopen(file_name, "r");
	if (file == NULL) {
		printf("[%s:%d] Failed to open %s\n", __FILE__, __LINE__, file_name);
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	length = (size_t)ftell(file);
	rewind(file);

	source_code = (char *)malloc(length + 1);
	fread(source_code, length, 1, file);
	source_code[length] = '\0';

	fclose(file);

	*len = length;
	return source_code;
}

double get_time() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (double)tv.tv_sec + (double)1e-6 * tv.tv_usec;
}

int main() {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	char *kernel_source;
	size_t kernel_source_size;
	cl_kernel kernel;
	cl_int err;

	//get platform
	err = clGetPlatformIDs(1,&platform,NULL);
	CHECK_ERROR(err);
	
	//get device
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_GPU,1,&device,NULL);
	CHECK_ERROR(err);
	
	//create context
	context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
	CHECK_ERROR(err);

	//create queue
	queue = clCreateCommandQueue(context,device,0,&err);
	CHECK_ERROR(err);

	//create program
	kernel_source = get_source_code("kernel.cl",&kernel_source_size);
	program = clCreateProgramWithSource(context,1,(const char**)&kernel_source,&kernel_source_size,&err);
	CHECK_ERROR(err);

	err = clBuildProgram(program,1,&device,"",NULL,NULL);
	if(err == CL_BUILD_PROGRAM_FAILURE) {
		size_t log_size;
		char *log;
		
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,0,NULL,&log_size);
		CHECK_ERROR(err);

		log = (char *)malloc(log_size+1);
		err = clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,log_size,log,NULL);
		CHECK_ERROR(err);

		log[log_size] = '\0';
		printf("Compiler error:\n%s\n",log);
		free(log);
		exit(0);
	}
	CHECK_ERROR(err);
	
	kernel = clCreateKernel(program,"vec_add",&err);
	CHECK_ERROR(err);
	
	int *A = (int*)malloc(sizeof(int) * 16384);
	int *B = (int*)malloc(sizeof(int) * 16384);
	int *C = (int*)malloc(sizeof(int) * 16384);
	int i;

	for(i=0;i<16384;i++) {
		A[i] = rand() % 100;
		B[i] = rand() % 100;
	}

	cl_mem bufA,bufB,bufC;
	
	bufA = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(int) * 16384,NULL,&err);
	CHECK_ERROR(err);

	bufB = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(int) * 16384,NULL,&err);
        CHECK_ERROR(err);

	bufC = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(int) * 16384,NULL,&err);
        CHECK_ERROR(err);

	err = clEnqueueWriteBuffer(queue,bufA,CL_FALSE,0,sizeof(int) * 16384,A,0,NULL,NULL);
	CHECK_ERROR(err);

	err = clEnqueueWriteBuffer(queue,bufB,CL_FALSE,0,sizeof(int) * 16384,B,0,NULL,NULL);
        CHECK_ERROR(err);

	err = clSetKernelArg(kernel,0,sizeof(cl_mem),&bufA);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,1,sizeof(cl_mem),&bufB);
        CHECK_ERROR(err);

	err = clSetKernelArg(kernel,2,sizeof(cl_mem),&bufC);
        CHECK_ERROR(err);

	size_t global_size = 16384;
	size_t local_size = 256;
	
	double start = get_time();

	err = clEnqueueNDRangeKernel(queue,kernel,1,NULL,&global_size,&local_size,0,NULL,NULL);
	CHECK_ERROR(err);

	err = clEnqueueReadBuffer(queue,bufC,CL_TRUE,0,sizeof(int) * 16384,C,0,NULL,NULL);
        CHECK_ERROR(err);

	for(i=0;i<16384;i++){
		if(A[i] + B[i] != C[i]) {
			printf("Verification failed! A[%d] = %d,B[%d] = %d,C[%d] = %d\n",i,A[i],i,B[i],i,C[i]);
			break;
		}
	}
	if(i == 16384) {
		printf("Verification success!\n");
	}
	
	double end = get_time();

	printf("second : %f\n",end-start);

	clReleaseMemObject(bufA);
	clReleaseMemObject(bufB);
	clReleaseMemObject(bufC);
	free(A);
	free(B);
	free(C);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	printf("Finished!\n");

	return 0;
}
