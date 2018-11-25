#include "OpenCLHelper.h"
#include "Common.h"
#include "Constants.h"

#include <vector>
#include <iostream>

#include "GL/glew.h"
#include "vector_types.h"

OpenCLHelper::OpenCLHelper()
{
	// Platform Check
	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, NULL, &platformIdCount);

	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), NULL);

	int gpuIdx = -1;
	int cpuIdx = -1;
	char platform_string[1024];
	for (uint32 i = 0; i < platformIds.size(); i++)
	{
		clGetPlatformInfo(platformIds.at(i), CL_PLATFORM_NAME, sizeof(platform_string), &platform_string, NULL);
		if (strcmp(platform_string, "NVIDIA CUDA") == 0)
		{
			gpuIdx = i;
			break;
		}
		else
		{
			cpuIdx = i;
		}
	}

	// Device Check
	cl_uint gpuIdCount = 0;
	clGetDeviceIDs(platformIds[gpuIdx], CL_DEVICE_TYPE_GPU, 0, NULL, &gpuIdCount);
	std::vector<cl_device_id> gpuIds(gpuIdCount);
	clGetDeviceIDs(platformIds[gpuIdx], CL_DEVICE_TYPE_GPU, gpuIdCount, gpuIds.data(), NULL);

	cl_uint cpuIdCount = 0;
	clGetDeviceIDs(platformIds[cpuIdx], CL_DEVICE_TYPE_CPU, 0, NULL, &cpuIdCount);
	std::vector<cl_device_id> cpuIds(gpuIdCount);
	clGetDeviceIDs(platformIds[cpuIdx], CL_DEVICE_TYPE_CPU, cpuIdCount, cpuIds.data(), NULL);

	std::cout << "GPU Device" << std::endl;
	char device_string[1024];
	clGetDeviceInfo(gpuIds[0], CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
	std::cout << "Device name: " << device_string << std::endl;

	char cOCLVersion[32];
	clGetDeviceInfo(gpuIds[0], CL_DEVICE_VERSION, sizeof(cOCLVersion), &cOCLVersion, 0);
	std::cout << "Device OCL Version: " << cOCLVersion << std::endl;

	std::cout << "CPU Device" << std::endl;
	clGetDeviceInfo(cpuIds[0], CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
	std::cout << "Device name: " << device_string << std::endl;

	clGetDeviceInfo(cpuIds[0], CL_DEVICE_VERSION, sizeof(cOCLVersion), &cOCLVersion, 0);
	std::cout << "Device OCL Version: " << cOCLVersion << std::endl;

	_gpuContext = clCreateContext(NULL, gpuIdCount, gpuIds.data(), NULL, NULL, &_error);
	CL_CHECK(_error);

	_gpuQueue = clCreateCommandQueue(_gpuContext, gpuIds[0], 0, &_error);
	CL_CHECK(_error);

	_cpuContext = clCreateContext(NULL, cpuIdCount, cpuIds.data(), NULL, NULL, &_error);
	CL_CHECK(_error);

	_cpuQueue = clCreateCommandQueue(_cpuContext, cpuIds[0], 0, &_error);
	CL_CHECK(_error);

	_computeKernel.reset(new Kernel(_gpuContext, gpuIds.data(), "computeForce.cl", "computeForce"));
	_integrateKernel.reset(new Kernel(_gpuContext, gpuIds.data(), "integrate.cl", "integrate"));

	_minMaxKernel.reset(new Kernel(_cpuContext, cpuIds.data(), "minMax.cl", "minMax"));

	_globalWorkSize = BLOCKSIZE * (ceil((float)(NUMBEROFGALAXIES * NUMBEROFSTARS) / (float)BLOCKSIZE));
	_localWorkSize = BLOCKSIZE;
}

void OpenCLHelper::allocMem()
{
	_gpuStarsMem = clCreateBuffer(_gpuContext, CL_MEM_READ_WRITE, NUMBEROFSTARS * NUMBEROFGALAXIES * sizeof(Star), NULL, &_error);
	CL_CHECK(_error);

	_cpuStarsMem = clCreateBuffer(_cpuContext, CL_MEM_READ_WRITE, NUMBEROFSTARS * NUMBEROFGALAXIES * sizeof(Star), NULL, &_error);
	CL_CHECK(_error);

	_nodesMem = clCreateBuffer(_gpuContext, CL_MEM_READ_WRITE, NUMBEROFNODES * sizeof(Node), NULL, &_error);
	CL_CHECK(_error);

	_minMem = clCreateBuffer(_cpuContext, CL_MEM_READ_WRITE, sizeof(cl_float3), NULL, &_error);
	CL_CHECK(_error);

	_maxMem = clCreateBuffer(_cpuContext, CL_MEM_READ_WRITE, sizeof(cl_float3), NULL, &_error);
	CL_CHECK(_error);
}

void OpenCLHelper::initKernelMem()
{
	_computeKernel->setKernelArg(0, &_gpuStarsMem);
	_computeKernel->setKernelArg(1, &_nodesMem);

	_integrateKernel->setKernelArg(0, &_gpuStarsMem);

	_minMaxKernel->setKernelArg(0, &_cpuStarsMem);
	_minMaxKernel->setKernelArg(1, &_minMem);
	_minMaxKernel->setKernelArg(2, &_maxMem);
}

void OpenCLHelper::uploadStarsGPU(Star* stars)
{
	_error = clEnqueueWriteBuffer(_gpuQueue, _gpuStarsMem, CL_FALSE, 0, NUMBEROFSTARS * NUMBEROFGALAXIES * sizeof(Star), stars, 0, NULL, NULL);
	CL_CHECK(_error);
}

void OpenCLHelper::uploadStarsCPU(Star* stars)
{
	_error = clEnqueueWriteBuffer(_cpuQueue, _cpuStarsMem, CL_TRUE, 0, NUMBEROFSTARS * NUMBEROFGALAXIES * sizeof(Star), stars, 0, NULL, NULL);
	CL_CHECK(_error);
}

void OpenCLHelper::uploadStepData(Node* nodes)
{
	_error = clEnqueueWriteBuffer(_gpuQueue, _nodesMem, CL_FALSE, 0, NUMBEROFNODES * sizeof(Node), nodes, 0, NULL, NULL);
	CL_CHECK(_error);
}

void OpenCLHelper::computeForceAndIntegrate(Star* stars)
{
	glFinish();

	_error = clFinish(_gpuQueue);
	CL_CHECK(_error);

	cl_event computeForceEvent, integrateEvent;
	_error = clEnqueueNDRangeKernel(_gpuQueue, _computeKernel->kernel, 1, NULL, &_globalWorkSize, &_localWorkSize, 0, NULL, &computeForceEvent);
	CL_CHECK(_error);
	_error = clEnqueueNDRangeKernel(_gpuQueue, _integrateKernel->kernel, 1, NULL, &_globalWorkSize, &_localWorkSize, 1, &computeForceEvent, &integrateEvent);
	CL_CHECK(_error);

	_error = clFinish(_gpuQueue);
	CL_CHECK(_error);

	// Downloading new star data
	_error = clEnqueueReadBuffer(_gpuQueue, _gpuStarsMem, CL_TRUE, 0, NUMBEROFSTARS * NUMBEROFGALAXIES * sizeof(Star), stars, 1, &integrateEvent, NULL);
	CL_CHECK(_error);
}

void OpenCLHelper::minMax(cl_float3* min, cl_float3* max)
{
	size_t globalWorkSize = 1;
	size_t localWorkSize = 1;

	_error = clEnqueueWriteBuffer(_cpuQueue, _minMem, CL_FALSE, 0, sizeof(cl_float3), min, 0, NULL, NULL);
	CL_CHECK(_error);
	_error = clEnqueueWriteBuffer(_cpuQueue, _maxMem, CL_FALSE, 0, sizeof(cl_float3), max, 0, NULL, NULL);
	CL_CHECK(_error);

	_error = clFinish(_cpuQueue);
	CL_CHECK(_error);

	_error = clEnqueueNDRangeKernel(_cpuQueue, _minMaxKernel->kernel, 1, NULL, &globalWorkSize, &localWorkSize, 0, NULL, NULL);
	CL_CHECK(_error);

	_error = clFinish(_cpuQueue);
	CL_CHECK(_error);

	_error = clEnqueueReadBuffer(_cpuQueue, _minMem, CL_FALSE, 0, sizeof(cl_float3), min, 0, NULL, NULL);
	CL_CHECK(_error);

	_error = clEnqueueReadBuffer(_cpuQueue, _maxMem, CL_FALSE, 0, sizeof(cl_float3), max, 0, NULL, NULL);
	CL_CHECK(_error);

	_error = clFinish(_cpuQueue);
	CL_CHECK(_error);
}

OpenCLHelper::~OpenCLHelper()
{
	clReleaseMemObject(_gpuStarsMem);
	clReleaseMemObject(_cpuStarsMem);
	clReleaseMemObject(_minMem);
	clReleaseMemObject(_maxMem);
	clReleaseMemObject(_nodesMem);

	clReleaseCommandQueue(_gpuQueue);
	clReleaseCommandQueue(_cpuQueue);
	clReleaseContext(_gpuContext);
	clReleaseContext(_cpuContext);
}
