#pragma once

#include "Types.h"
#include "Kernel.h"

#include <vector>
#include <string>

#include "CL/cl.h"

class Star;
class Node;

class OpenCLHelper
{
public:
	OpenCLHelper();

	void allocMem();
	void initKernelMem();

	template <typename T>
	void setComputeArg(uint32 index, size_t size, T& data) { _computeKernel->setScalarKernelArg(index, size, data); }

	template <typename T>
	void setIntegrateArg(uint32 index, size_t size, T& data) { _integrateKernel->setScalarKernelArg(index, size, data); }

	template <typename T>
	void setCpuArg(uint32 index, size_t size, T& data) { _minMaxKernel->setScalarKernelArg(index, size, data); }

	void uploadStarsGPU(Star* stars);
	void uploadStarsCPU(Star* stars);
	void uploadStepData(Node* nodes);

	void computeForceAndIntegrate(Star* stars);
	void minMax(cl_float3* min, cl_float3* max);

	~OpenCLHelper();
private:
	cl_int _error;

	// GPU
	cl_context _gpuContext; // GPU context
	cl_mem _gpuStarsMem;
	cl_mem _nodesMem;
	cl_command_queue _gpuQueue;
	std::unique_ptr<Kernel> _computeKernel;
	std::unique_ptr<Kernel> _integrateKernel;

	// CPU
	cl_context _cpuContext; // CPU context
	cl_mem _cpuStarsMem;
	cl_mem _minMem;
	cl_mem _maxMem;
	cl_command_queue _cpuQueue;
	std::unique_ptr<Kernel> _minMaxKernel;

	size_t _globalWorkSize;
	size_t _localWorkSize;
};

