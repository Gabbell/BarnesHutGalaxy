#pragma once

#include "Types.h"

#include <string>
#include <cassert>|

#include "CL/cl.h"

#define CL_CHECK(error) assert(error == CL_SUCCESS); 

class Kernel
{
public:
	Kernel();
	Kernel(cl_context& context, cl_device_id* deviceId, std::string name, std::string kernelName);
	~Kernel();

	template <typename T>
	void setScalarKernelArg(uint32 index, size_t size, T& data)
	{
		_error = clSetKernelArg(kernel, index, size, (void*)&data);
		CL_CHECK(_error);
	}

	void setKernelArg(uint32 index, cl_mem* data)
	{
		_error = clSetKernelArg(kernel, index, sizeof(cl_mem), (void*)data);
		CL_CHECK(_error);
	}

	cl_kernel kernel;

private:
	cl_int _error;
	cl_program _program;

	std::string _name;
	std::string _kernelString;
};

