#pragma once
#include <array>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/core/hal/intrin.hpp"
#include "CL/cl.h"
#include "utility.hpp"

using std::string;
using std::ifstream;

#pragma warning (disable : 4996)
#pragma OPENCL EXTENSION all : enable

static const char* clprogram = "\n";

static void clPrintDevInfo(cl_device_id device) {
	char device_string[512];

	// CL_DEVICE_NAME
	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
	printf("  CL_DEVICE_NAME: \t\t\t%s\n", device_string);

	// CL_DEVICE_VENDOR
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
	printf("  CL_DEVICE_VENDOR: \t\t\t%s\n", device_string);

	// CL_DRIVER_VERSION
	clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
	printf("  CL_DRIVER_VERSION: \t\t\t%s\n", device_string);

	// CL_DEVICE_INFO
	cl_device_type type;
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	if (type & CL_DEVICE_TYPE_CPU)
		printf("  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_CPU");
	if (type & CL_DEVICE_TYPE_GPU)
		printf("  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_GPU");
	if (type & CL_DEVICE_TYPE_ACCELERATOR)
		printf("  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_ACCELERATOR");
	if (type & CL_DEVICE_TYPE_DEFAULT)
		printf("  CL_DEVICE_TYPE:\t\t\t%s\n", "CL_DEVICE_TYPE_DEFAULT");

	// CL_DEVICE_MAX_COMPUTE_UNITS
	cl_uint compute_units;
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	printf("  CL_DEVICE_MAX_COMPUTE_UNITS:\t\t%u\n", compute_units);

	// CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
	size_t workitem_dims;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);
	printf("  CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:\t%u\n", workitem_dims);

	// CL_DEVICE_MAX_WORK_ITEM_SIZES
	size_t workitem_size[3];
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
	printf("  CL_DEVICE_MAX_WORK_ITEM_SIZES:\t%u / %u / %u \n", workitem_size[0], workitem_size[1], workitem_size[2]);

	// CL_DEVICE_MAX_WORK_GROUP_SIZE
	size_t workgroup_size;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
	printf("  CL_DEVICE_MAX_WORK_GROUP_SIZE:\t%u\n", workgroup_size);

	// CL_DEVICE_MAX_CLOCK_FREQUENCY
	cl_uint clock_frequency;
	clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
	printf("  CL_DEVICE_MAX_CLOCK_FREQUENCY:\t%u MHz\n", clock_frequency);

	// CL_DEVICE_ADDRESS_BITS
	cl_uint addr_bits;
	clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(addr_bits), &addr_bits, NULL);
	printf("  CL_DEVICE_ADDRESS_BITS:\t\t%u\n", addr_bits);

	// CL_DEVICE_MAX_MEM_ALLOC_SIZE
	cl_ulong max_mem_alloc_size;
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(max_mem_alloc_size), &max_mem_alloc_size, NULL);
	printf("  CL_DEVICE_MAX_MEM_ALLOC_SIZE:\t\t%u MByte\n", (unsigned int)(max_mem_alloc_size / (1024 * 1024)));

	// CL_DEVICE_GLOBAL_MEM_SIZE
	cl_ulong mem_size;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	printf("  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t%u MByte\n", (unsigned int)(mem_size / (1024 * 1024)));

	// CL_DEVICE_ERROR_CORRECTION_SUPPORT
	cl_bool error_correction_support;
	clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(error_correction_support), &error_correction_support, NULL);
	printf("  CL_DEVICE_ERROR_CORRECTION_SUPPORT:\t%s\n", error_correction_support == CL_TRUE ? "yes" : "no");

	// CL_DEVICE_LOCAL_MEM_TYPE
	cl_device_local_mem_type local_mem_type;
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL);
	printf("  CL_DEVICE_LOCAL_MEM_TYPE:\t\t%s\n", local_mem_type == 1 ? "local" : "global");

	// CL_DEVICE_LOCAL_MEM_SIZE
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	printf("  CL_DEVICE_LOCAL_MEM_SIZE:\t\t%u KByte\n", (unsigned int)(mem_size / 1024));

	// CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
	clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(mem_size), &mem_size, NULL);
	printf("  CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:\t%u KByte\n", (unsigned int)(mem_size / 1024));

	// CL_DEVICE_QUEUE_PROPERTIES
	cl_command_queue_properties queue_properties;
	clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queue_properties), &queue_properties, NULL);
	if (queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
		printf("  CL_DEVICE_QUEUE_PROPERTIES:\t\t%s\n", "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE");
	if (queue_properties & CL_QUEUE_PROFILING_ENABLE)
		printf("  CL_DEVICE_QUEUE_PROPERTIES:\t\t%s\n", "CL_QUEUE_PROFILING_ENABLE");

	// CL_DEVICE_IMAGE_SUPPORT
	cl_bool image_support;
	clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, NULL);
	printf("  CL_DEVICE_IMAGE_SUPPORT:\t\t%u\n", image_support);

	// CL_DEVICE_MAX_READ_IMAGE_ARGS
	cl_uint max_read_image_args;
	clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(max_read_image_args), &max_read_image_args, NULL);
	printf("  CL_DEVICE_MAX_READ_IMAGE_ARGS:\t%u\n", max_read_image_args);

	// CL_DEVICE_MAX_WRITE_IMAGE_ARGS
	cl_uint max_write_image_args;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(max_write_image_args), &max_write_image_args, NULL);
	printf("  CL_DEVICE_MAX_WRITE_IMAGE_ARGS:\t%u\n", max_write_image_args);

	// CL_DEVICE_IMAGE2D_MAX_WIDTH, CL_DEVICE_IMAGE2D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_DEPTH
	size_t szMaxDims[5];
	printf("\n  CL_DEVICE_IMAGE <dim>");
	clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &szMaxDims[0], NULL);
	printf("\t\t\t2D_MAX_WIDTH\t %u\n", szMaxDims[0]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[1], NULL);
	printf("\t\t\t\t\t2D_MAX_HEIGHT\t %u\n", szMaxDims[1]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &szMaxDims[2], NULL);
	printf("\t\t\t\t\t3D_MAX_WIDTH\t %u\n", szMaxDims[2]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &szMaxDims[3], NULL);
	printf("\t\t\t\t\t3D_MAX_HEIGHT\t %u\n", szMaxDims[3]);
	clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &szMaxDims[4], NULL);
	printf("\t\t\t\t\t3D_MAX_DEPTH\t %u\n", szMaxDims[4]);

	// CL_DEVICE_PREFERRED_VECTOR_WIDTH_<type>
	printf("  CL_DEVICE_PREFERRED_VECTOR_WIDTH_<t>\t");
	cl_uint vec_width[6];
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &vec_width[0], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &vec_width[1], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &vec_width[2], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &vec_width[3], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &vec_width[4], NULL);
	clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &vec_width[5], NULL);
	printf("CHAR %u, SHORT %u, INT %u, FLOAT %u, DOUBLE %u\n\n\n",
		vec_width[0], vec_width[1], vec_width[2], vec_width[3], vec_width[4]);
}

namespace gxr
{
	class OCL
	{
	public:
		int64_t tickprev;

		void timer_start()
		{
			tickprev = getTickCount();
		}

		void timer_end(char const* msg)
		{
			int64_t ticksum = getTickCount() - tickprev;
			double curt = 1e3 * ticksum / getTickFrequency();
			GXR_Printf("%9.3f | %s\n", curt, msg);
		}

		const char* getErrorString(cl_int error)
		{
			switch (error) {
				// run-time and JIT compiler errors
			case 0: return "CL_SUCCESS";
			case -1: return "CL_DEVICE_NOT_FOUND";
			case -2: return "CL_DEVICE_NOT_AVAILABLE";
			case -3: return "CL_COMPILER_NOT_AVAILABLE";
			case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
			case -5: return "CL_OUT_OF_RESOURCES";
			case -6: return "CL_OUT_OF_HOST_MEMORY";
			case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
			case -8: return "CL_MEM_COPY_OVERLAP";
			case -9: return "CL_IMAGE_FORMAT_MISMATCH";
			case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
			case -11: return "CL_BUILD_PROGRAM_FAILURE";
			case -12: return "CL_MAP_FAILURE";
			case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
			case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
			case -15: return "CL_COMPILE_PROGRAM_FAILURE";
			case -16: return "CL_LINKER_NOT_AVAILABLE";
			case -17: return "CL_LINK_PROGRAM_FAILURE";
			case -18: return "CL_DEVICE_PARTITION_FAILED";
			case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

				// compile-time errors
			case -30: return "CL_INVALID_VALUE";
			case -31: return "CL_INVALID_DEVICE_TYPE";
			case -32: return "CL_INVALID_PLATFORM";
			case -33: return "CL_INVALID_DEVICE";
			case -34: return "CL_INVALID_CONTEXT";
			case -35: return "CL_INVALID_QUEUE_PROPERTIES";
			case -36: return "CL_INVALID_COMMAND_QUEUE";
			case -37: return "CL_INVALID_HOST_PTR";
			case -38: return "CL_INVALID_MEM_OBJECT";
			case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
			case -40: return "CL_INVALID_IMAGE_SIZE";
			case -41: return "CL_INVALID_SAMPLER";
			case -42: return "CL_INVALID_BINARY";
			case -43: return "CL_INVALID_BUILD_OPTIONS";
			case -44: return "CL_INVALID_PROGRAM";
			case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
			case -46: return "CL_INVALID_KERNEL_NAME";
			case -47: return "CL_INVALID_KERNEL_DEFINITION";
			case -48: return "CL_INVALID_KERNEL";
			case -49: return "CL_INVALID_ARG_INDEX";
			case -50: return "CL_INVALID_ARG_VALUE";
			case -51: return "CL_INVALID_ARG_SIZE";
			case -52: return "CL_INVALID_KERNEL_ARGS";
			case -53: return "CL_INVALID_WORK_DIMENSION";
			case -54: return "CL_INVALID_WORK_GROUP_SIZE";
			case -55: return "CL_INVALID_WORK_ITEM_SIZE";
			case -56: return "CL_INVALID_GLOBAL_OFFSET";
			case -57: return "CL_INVALID_EVENT_WAIT_LIST";
			case -58: return "CL_INVALID_EVENT";
			case -59: return "CL_INVALID_OPERATION";
			case -60: return "CL_INVALID_GL_OBJECT";
			case -61: return "CL_INVALID_BUFFER_SIZE";
			case -62: return "CL_INVALID_MIP_LEVEL";
			case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
			case -64: return "CL_INVALID_PROPERTY";
			case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
			case -66: return "CL_INVALID_COMPILER_OPTIONS";
			case -67: return "CL_INVALID_LINKER_OPTIONS";
			case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

				// extension errors
			case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
			case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
			case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
			case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
			case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
			case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
			default: return "Unknown OpenCL error";
			}
		}

	public:
		cl_platform_id platform;
		cl_device_id device;
		cl_context context;
		cl_program program;
		cl_kernel kernel_spafilter;
		cl_kernel kernel_tnr_pre;
		cl_kernel kernel_tnr_post;
		cl_kernel kernel_ydenoise;
		cl_kernel kernel_ydown;
		cl_kernel kernel_yupmerge;
		cl_kernel kernel_cdenoise;
		cl_kernel kernel_cdown;
		cl_kernel kernel_cmerge;
		cl_kernel kernel_dm_pre;
		cl_kernel kernel_dm_post;
		cl_command_queue queue;

		OCL()
		{
			cl_int err;
			size_t sz, log_sz;
			err = clGetPlatformIDs(1, &platform, NULL);
			if (err < 0)
			{
				GXR_Printf("Fail to identify a platform\n");
				CV_Assert(err >= 0);
			}

			err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
			if (err < 0)
			{
				GXR_Printf("Fail to access GPU\n");
				CV_Assert(err >= 0);
			}

			clPrintDevInfo(device);

			context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create a context\n");
				CV_Assert(err >= 0);
			}

			FILE* fp = fopen("clprogram.cpp", "rb");
			CV_Assert(fp);
			fseek(fp, 0, SEEK_END);
			sz = ftell(fp);
			rewind(fp);
			char* programstr = (char*)malloc(sz + 1);
			programstr[sz] = 0;
			fread(programstr, 1, sz, fp);
			program = clCreateProgramWithSource(context, 1, const_cast<const char**>(&programstr), &sz, &err);
			free(programstr);
			if (err < 0)
			{
				GXR_Printf("Fail to create the program\n");
				//CV_Assert(err >= 0);
			}

			err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
			if (err < 0)
			{
				clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_sz);
				char* log = (char*)malloc(log_sz + 1);
				log[log_sz] = '\0';
				clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_sz + 1, log, NULL);
				GXR_Printf("%s\n", log);
				free(log);
				//CV_Assert(err >= 0);
			}

			queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create a command queue\n");
				CV_Assert(err >= 0);
			}

			kernel_spafilter = clCreateKernel(program, "spafilter", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel spafilter\n");
				//CV_Assert(err >= 0);
			}

			kernel_tnr_pre = clCreateKernel(program, "tnr_pre", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel tnr_pre\n");
				//CV_Assert(err >= 0);
			}

			kernel_tnr_post = clCreateKernel(program, "tnr_post", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel tnr_post\n");
				//CV_Assert(err >= 0);
			}

			kernel_ydenoise = clCreateKernel(program, "ydenoise", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel ydenoise\n");
				//CV_Assert(err >= 0);
			}

			kernel_ydown = clCreateKernel(program, "ydown", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel ydown\n");
				//CV_Assert(err >= 0);
			}

			kernel_yupmerge = clCreateKernel(program, "yupmerge", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel yupmerge\n");
				//CV_Assert(err >= 0);
			}

			kernel_cdenoise = clCreateKernel(program, "cdenoise", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel cdenoise\n");
				//CV_Assert(err >= 0);
			}

			kernel_cdown = clCreateKernel(program, "cdown", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel cdown\n");
				//CV_Assert(err >= 0);
			}

			kernel_cmerge = clCreateKernel(program, "cmerge", &err);
			if (err < 0)
			{
				GXR_Printf("Fail to create kernel cmerge\n");
				//CV_Assert(err >= 0);
			}
		}

		~OCL()
		{
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
			clReleaseKernel(kernel_spafilter);
			clReleaseKernel(kernel_tnr_pre);
			clReleaseKernel(kernel_tnr_post);
			clReleaseKernel(kernel_ydenoise);
			clReleaseKernel(kernel_ydown);
			clReleaseKernel(kernel_cdenoise);
			clReleaseKernel(kernel_cdown);
			clReleaseKernel(kernel_cmerge);
			clReleaseProgram(program);
		}
	};
}
