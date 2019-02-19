#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define CHECK_ERROR(err) \
	if (err != CL_SUCCESS) { \
		printf("[%s:%d] OpenCL error %d\n", __FILE__, __LINE__, err); \
		exit(EXIT_FAILURE); \
	}

int main() {
	//platform v
	cl_uint num_platforms;
	cl_platform_id *platforms;

	//device v
	cl_uint num_devices;
	cl_device_id *devices;

	char str[1024];

	cl_device_type device_type;
	size_t max_work_group_size;

	cl_ulong global_mem_size;
	cl_ulong local_mem_size;
	cl_ulong max_mem_alloc_size;

	cl_uint p,d;
	cl_int err;

	//get num of platform
	err = clGetPlatformIDs(0,NULL,&num_platforms);
	CHECK_ERROR(err);

	//get platforms
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
	err = clGetPlatformIDs(num_platforms,platforms,NULL);
	CHECK_ERROR(err);	

	//print num of platform
	printf("Number of platforms: %u\n\n",num_platforms);
	//info of platforms
	for(p=0;p<num_platforms;p++) {
		printf("platform: %u\n",p);
		//platform name
		err = clGetPlatformInfo(platforms[p],CL_PLATFORM_NAME,1024,str,NULL);
		CHECK_ERROR(err);
		printf("- platform name : %s\n",str);
		//platform vender
		err = clGetPlatformInfo(platforms[p],CL_PLATFORM_VENDOR,1024,str,NULL);
		CHECK_ERROR(err);
		printf("- platform vender : %s\n",str);
		//get num of device
		err = clGetDeviceIDs(platforms[p],CL_DEVICE_TYPE_ALL,0,NULL,&num_devices);
		CHECK_ERROR(err);
		//print num of device
		printf("Number of devices : %u\n\n",num_devices);
		//get devices
		devices = (cl_device_id*)malloc(sizeof(cl_device_id)*num_devices);
		err = clGetDeviceIDs(platforms[p],CL_DEVICE_TYPE_ALL,num_devices,devices,NULL);
		CHECK_ERROR(err);

		//info of devices
		for(d=0;d<num_devices;d++){
			printf("device: %u\n",d);

			err = clGetDeviceInfo(devices[d],CL_DEVICE_TYPE,sizeof(cl_device_type),&device_type,NULL);
			CHECK_ERROR(err);
			printf("- device type :");
			if(device_type & CL_DEVICE_TYPE_CPU) printf("CPU");
			if(device_type & CL_DEVICE_TYPE_GPU) printf("GPU");
			if(device_type & CL_DEVICE_TYPE_ACCELERATOR) printf("ACCELERATOR");
			if(device_type & CL_DEVICE_TYPE_DEFAULT) printf("DEFAULT");
			if(device_type & CL_DEVICE_TYPE_CUSTOM) printf("CUSTOM");
			printf("\n");

			err = clGetDeviceInfo(devices[d],CL_DEVICE_NAME,1024,str,NULL);
			CHECK_ERROR(err);
			printf("- device name : %s\n",str);

			err = clGetDeviceInfo(devices[d],CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(size_t),
					&max_work_group_size,NULL);
			CHECK_ERROR(err);
			printf("- device max group size : %lu\n",max_work_group_size);

			err = clGetDeviceInfo(devices[d],CL_DEVICE_GLOBAL_MEM_SIZE,sizeof(cl_ulong),
					&global_mem_size,NULL);
			CHECK_ERROR(err);
			printf("- device global mem size : %lu\n",global_mem_size);

			err = clGetDeviceInfo(devices[d],CL_DEVICE_LOCAL_MEM_SIZE,sizeof(cl_ulong),
					&local_mem_size,NULL);
			CHECK_ERROR(err);
			printf("- device local mem size : %lu\n",local_mem_size);

			err = clGetDeviceInfo(devices[d],CL_DEVICE_MAX_MEM_ALLOC_SIZE,
					sizeof(cl_ulong),&max_mem_alloc_size,NULL);
			CHECK_ERROR(err);
			printf("- device max alloc size : %lu\n",max_mem_alloc_size);
		}

		free(devices);
	}	

	free(platforms);
	return 0;

	return 0;
}
