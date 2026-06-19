/* ************************************************************************
 * Copyright 2013-2015 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/

#define clfftVersionMajor 2
#define clfftVersionMinor 14
#define clfftVersionPatch 0

enum clfftStatus_
{
	CLFFT_INVALID_GLOBAL_WORK_SIZE = CL_INVALID_GLOBAL_WORK_SIZE,
	CLFFT_INVALID_MIP_LEVEL = CL_INVALID_MIP_LEVEL,
	CLFFT_INVALID_BUFFER_SIZE = CL_INVALID_BUFFER_SIZE,
	CLFFT_INVALID_GL_OBJECT = CL_INVALID_GL_OBJECT,
	CLFFT_INVALID_OPERATION = CL_INVALID_OPERATION,
	CLFFT_INVALID_EVENT = CL_INVALID_EVENT,
	CLFFT_INVALID_EVENT_WAIT_LIST = CL_INVALID_EVENT_WAIT_LIST,
	CLFFT_INVALID_GLOBAL_OFFSET = CL_INVALID_GLOBAL_OFFSET,
	CLFFT_INVALID_WORK_ITEM_SIZE = CL_INVALID_WORK_ITEM_SIZE,
	CLFFT_INVALID_WORK_GROUP_SIZE = CL_INVALID_WORK_GROUP_SIZE,
	CLFFT_INVALID_WORK_DIMENSION = CL_INVALID_WORK_DIMENSION,
	CLFFT_INVALID_KERNEL_ARGS = CL_INVALID_KERNEL_ARGS,
	CLFFT_INVALID_ARG_SIZE = CL_INVALID_ARG_SIZE,
	CLFFT_INVALID_ARG_VALUE = CL_INVALID_ARG_VALUE,
	CLFFT_INVALID_ARG_INDEX = CL_INVALID_ARG_INDEX,
	CLFFT_INVALID_KERNEL = CL_INVALID_KERNEL,
	CLFFT_INVALID_KERNEL_DEFINITION = CL_INVALID_KERNEL_DEFINITION,
	CLFFT_INVALID_KERNEL_NAME = CL_INVALID_KERNEL_NAME,
	CLFFT_INVALID_PROGRAM_EXECUTABLE = CL_INVALID_PROGRAM_EXECUTABLE,
	CLFFT_INVALID_PROGRAM = CL_INVALID_PROGRAM,
	CLFFT_INVALID_BUILD_OPTIONS = CL_INVALID_BUILD_OPTIONS,
	CLFFT_INVALID_BINARY = CL_INVALID_BINARY,
	CLFFT_INVALID_SAMPLER = CL_INVALID_SAMPLER,
	CLFFT_INVALID_IMAGE_SIZE = CL_INVALID_IMAGE_SIZE,
	CLFFT_INVALID_IMAGE_FORMAT_DESCRIPTOR = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,
	CLFFT_INVALID_MEM_OBJECT = CL_INVALID_MEM_OBJECT,
	CLFFT_INVALID_HOST_PTR = CL_INVALID_HOST_PTR,
	CLFFT_INVALID_COMMAND_QUEUE = CL_INVALID_COMMAND_QUEUE,
	CLFFT_INVALID_QUEUE_PROPERTIES = CL_INVALID_QUEUE_PROPERTIES,
	CLFFT_INVALID_CONTEXT = CL_INVALID_CONTEXT,
	CLFFT_INVALID_DEVICE = CL_INVALID_DEVICE,
	CLFFT_INVALID_PLATFORM = CL_INVALID_PLATFORM,
	CLFFT_INVALID_DEVICE_TYPE = CL_INVALID_DEVICE_TYPE,
	CLFFT_INVALID_VALUE = CL_INVALID_VALUE,
	CLFFT_MAP_FAILURE = CL_MAP_FAILURE,
	CLFFT_BUILD_PROGRAM_FAILURE = CL_BUILD_PROGRAM_FAILURE,
	CLFFT_IMAGE_FORMAT_NOT_SUPPORTED = CL_IMAGE_FORMAT_NOT_SUPPORTED,
	CLFFT_IMAGE_FORMAT_MISMATCH = CL_IMAGE_FORMAT_MISMATCH,
	CLFFT_MEM_COPY_OVERLAP = CL_MEM_COPY_OVERLAP,
	CLFFT_PROFILING_INFO_NOT_AVAILABLE = CL_PROFILING_INFO_NOT_AVAILABLE,
	CLFFT_OUT_OF_HOST_MEMORY = CL_OUT_OF_HOST_MEMORY,
	CLFFT_OUT_OF_RESOURCES = CL_OUT_OF_RESOURCES,
	CLFFT_MEM_OBJECT_ALLOCATION_FAILURE = CL_MEM_OBJECT_ALLOCATION_FAILURE,
	CLFFT_COMPILER_NOT_AVAILABLE = CL_COMPILER_NOT_AVAILABLE,
	CLFFT_DEVICE_NOT_AVAILABLE = CL_DEVICE_NOT_AVAILABLE,
	CLFFT_DEVICE_NOT_FOUND = CL_DEVICE_NOT_FOUND,
	CLFFT_SUCCESS = CL_SUCCESS,
	CLFFT_BUGCHECK = 4 * 1024,
	CLFFT_NOTIMPLEMENTED,
	CLFFT_TRANSPOSED_NOTIMPLEMENTED,
	CLFFT_FILE_NOT_FOUND,
	CLFFT_FILE_CREATE_FAILURE,
	CLFFT_VERSION_MISMATCH,
	CLFFT_INVALID_PLAN,
	CLFFT_DEVICE_NO_DOUBLE,
	CLFFT_DEVICE_MISMATCH,
	CLFFT_ENDSTATUS
};
typedef enum clfftStatus_ clfftStatus;

typedef enum clfftDim_
{
	CLFFT_1D = 1,
	CLFFT_2D,
	CLFFT_3D,
	ENDDIMENSION
} clfftDim;

typedef enum clfftLayout_
{
	CLFFT_COMPLEX_INTERLEAVED = 1,
	CLFFT_COMPLEX_PLANAR,
	CLFFT_HERMITIAN_INTERLEAVED,
	CLFFT_HERMITIAN_PLANAR,
	CLFFT_REAL,
	ENDLAYOUT
} clfftLayout;

typedef enum clfftPrecision_
{
	CLFFT_SINGLE = 1,
	CLFFT_DOUBLE,
	CLFFT_SINGLE_FAST,
	CLFFT_DOUBLE_FAST,
	ENDPRECISION
} clfftPrecision;

typedef enum clfftDirection_
{
	CLFFT_FORWARD = -1,
	CLFFT_BACKWARD = 1,
	CLFFT_MINUS = -1,
	CLFFT_PLUS = 1,
	ENDDIRECTION
} clfftDirection;

typedef enum clfftResultLocation_
{
	CLFFT_INPLACE = 1,
	CLFFT_OUTOFPLACE,
	ENDPLACE
} clfftResultLocation;

typedef enum clfftResultTransposed_
{
	CLFFT_NOTRANSPOSE = 1,
	CLFFT_TRANSPOSED,
	ENDTRANSPOSED
} clfftResultTransposed;

#define CLFFT_DUMP_PROGRAMS 0x1

struct clfftSetupData_
{
	cl_uint major;
	cl_uint minor;
	cl_uint patch;
	cl_ulong debugFlags;
};
typedef struct clfftSetupData_ clfftSetupData;

typedef enum clfftCallbackType_
{
	PRECALLBACK,
	POSTCALLBACK
} clfftCallbackType;

typedef size_t clfftPlanHandle;

extern clfftStatus clfftGetVersion(cl_uint *major, cl_uint *minor, cl_uint *patch);
extern clfftStatus clfftInitSetupData(clfftSetupData *setupData);
extern clfftStatus clfftSetup(const clfftSetupData *setupData);
extern clfftStatus clfftTeardown(void);

extern clfftStatus clfftCreateDefaultPlan(clfftPlanHandle *plHandle, cl_context context, const clfftDim dim, const size_t *clLengths);

extern clfftStatus clfftSetPlanPrecision(clfftPlanHandle plHandle, clfftPrecision precision);
extern clfftStatus clfftSetLayout(clfftPlanHandle plHandle, clfftLayout iLayout, clfftLayout oLayout);
extern clfftStatus clfftSetResultLocation(clfftPlanHandle plHandle, clfftResultLocation placeness);
extern clfftStatus clfftSetPlanScale(clfftPlanHandle plHandle, clfftDirection dir, cl_float scale);

extern clfftStatus clfftBakePlan(clfftPlanHandle plHandle, cl_uint numQueues, cl_command_queue *commQueueFFT,
	void(CL_CALLBACK *pfn_notify)(clfftPlanHandle plHandle, void *user_data), void *user_data);

extern clfftStatus clfftGetTmpBufSize(const clfftPlanHandle plHandle, size_t *buffersize);
extern clfftStatus clfftDestroyPlan(clfftPlanHandle *plHandle);

extern clfftStatus clfftEnqueueTransform(clfftPlanHandle plHandle, clfftDirection dir, cl_uint numQueuesAndEvents, cl_command_queue *commQueues, cl_uint numWaitEvents,
	const cl_event *waitEvents, cl_event *outEvents, cl_mem *inputBuffers, cl_mem *outputBuffers, cl_mem tmpBuffer);
