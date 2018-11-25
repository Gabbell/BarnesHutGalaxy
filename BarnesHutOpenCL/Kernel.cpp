#include "Kernel.h"

#include <fstream>
#include <sstream>
#include <iostream>

Kernel::Kernel()
{}

Kernel::Kernel(cl_context& context, cl_device_id* deviceId, std::string name, std::string kernelName)
	: _name(name)
{
	std::ifstream file(name);
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	_kernelString = buffer.str();

	const char* kernelString = _kernelString.c_str(); // Needed for proper casting
	_program = clCreateProgramWithSource(context, 1, &kernelString, NULL, &_error);
	CL_CHECK(_error);

	// Build program
	_error = clBuildProgram(_program, 1, deviceId, NULL, NULL, NULL);
	// Catch syntax errors in the Kernel code
	if (_error != CL_SUCCESS) {
		// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(_program, *deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		// Allocate memory for the log
		char *log = (char *)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(_program, *deviceId, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		std::cout << log << std::endl;

		free(log);
	}
	CL_CHECK(_error);

	kernel = clCreateKernel(_program, kernelName.c_str(), &_error);
	CL_CHECK(_error);
}

Kernel::~Kernel()
{
	clReleaseKernel(kernel);
	clReleaseProgram(_program);
}
