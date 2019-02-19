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

double get_time();

void mat_mul_opencl(float *A,float *B,float *C,int ROW_A,int COL_A,int COL_B) {
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
	
	kernel = clCreateKernel(program,"mat_mul",&err);
	CHECK_ERROR(err);

	cl_mem bufA,bufB,bufC;
	
	bufA = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(float)*ROW_A*COL_A,NULL,&err);
	CHECK_ERROR(err);

	bufB = clCreateBuffer(context,CL_MEM_READ_ONLY,sizeof(float)*COL_A*COL_B,NULL,&err);
        CHECK_ERROR(err);

	bufC = clCreateBuffer(context,CL_MEM_READ_WRITE,sizeof(float)*ROW_A*COL_B,NULL,&err);
        CHECK_ERROR(err);
	
	double start = get_time();

	err = clEnqueueWriteBuffer(queue,bufA,CL_FALSE,0,sizeof(float)*ROW_A*COL_A,A,0,NULL,NULL);
	CHECK_ERROR(err);

	err = clEnqueueWriteBuffer(queue,bufB,CL_FALSE,0,sizeof(float)*COL_A*COL_B,B,0,NULL,NULL);
        CHECK_ERROR(err);

	err = clSetKernelArg(kernel,0,sizeof(cl_mem),&bufA);
	CHECK_ERROR(err);

	err = clSetKernelArg(kernel,1,sizeof(cl_mem),&bufB);
        CHECK_ERROR(err);

	err = clSetKernelArg(kernel,2,sizeof(cl_mem),&bufC);
        CHECK_ERROR(err);

	err = clSetKernelArg(kernel,3,sizeof(cl_int),&ROW_A);
        CHECK_ERROR(err);

        err = clSetKernelArg(kernel,4,sizeof(cl_int),&COL_A);
        CHECK_ERROR(err);

        err = clSetKernelArg(kernel,5,sizeof(cl_int),&COL_B);
        CHECK_ERROR(err);

	size_t global_size[2] = {COL_B,ROW_A};
	size_t local_size[2] = {16,16};

	global_size[0] = (global_size[0] + local_size[0] - 1) / local_size[0] * local_size[0];
	global_size[1] = (global_size[1] + local_size[1] - 1) / local_size[1] * local_size[1];

	err = clEnqueueNDRangeKernel(queue,kernel,2,NULL,global_size,local_size,0,NULL,NULL);
	CHECK_ERROR(err);

	err = clEnqueueReadBuffer(queue,bufC,CL_TRUE,0,sizeof(float)*ROW_A*COL_B,C,0,NULL,NULL);
        CHECK_ERROR(err);

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
}
