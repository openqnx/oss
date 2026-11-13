/* opencl-headers: the Khronos OpenCL headers, compiled for OpenCL 3.0 and
 * used without any OpenCL library (the component is header only).
 *  - cl_half.h, the only code in the headers: float -> half conversion in
 *    the four rounding modes against IEEE 754 binary16 bit patterns
 *    (1.0 = 0x3C00, 0.1 = 0x2E66 to nearest and 0x2E67 upwards, the tie
 *    1 + 2^-11 to even, overflow to infinity or to the largest finite value
 *    depending on the mode, the smallest subnormal 2^-24 = 0x0001), from
 *    double, and half -> float back;
 *  - cl_platform.h: the scalar and vector types have the sizes and the
 *    alignment the OpenCL C types have on the device (cl_long 8, cl_half 2,
 *    cl_float4 16 bytes aligned to 16, cl_double8 64);
 *  - cl_version.h / cl.h: CL_TARGET_OPENCL_VERSION 300 enables the 3.0 API
 *    (CL_VERSION_3_0, cl_name_version of 4 + 64 bytes), CL_MAKE_VERSION packs
 *    major.minor.patch as 10.10.12 bits and the CL_VERSION_* macros unpack it;
 *  - the error codes and bit fields programs compare against;
 *  - cl_icd.h: the ICD dispatch table is the binary interface between the
 *    loader and every driver -- 150 entries, with clGetPlatformInfo,
 *    clCreateBuffer, clBuildProgram, clEnqueueNDRangeKernel,
 *    clCreateSubBuffer (first 1.1 entry), clCreateImage (1.2),
 *    clCreateCommandQueueWithProperties (2.0), clCloneKernel (2.1) and
 *    clCreateBufferWithProperties (3.0) at their fixed slots. */
#define CL_TARGET_OPENCL_VERSION 300
#include "common.h"
#include <stddef.h>
#include <math.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_icd.h>
#include <CL/cl_half.h>
#include <CL/opencl.h>

#define SLOT(f) (offsetof(cl_icd_dispatch, f) / sizeof(void *))

int main(void) {
  QT_BEGIN("opencl-headers");

  /* cl_half.h */
  QT_CHECK(cl_half_from_float(1.0f, CL_HALF_RTE) == 0x3C00, "half(1.0) = 0x3C00");
  QT_CHECK(cl_half_from_float(0.5f, CL_HALF_RTE) == 0x3800, "half(0.5) = 0x3800");
  QT_CHECK(cl_half_from_float(-2.0f, CL_HALF_RTE) == 0xC000, "half(-2.0) = 0xC000");
  QT_CHECK(cl_half_from_float(65504.0f, CL_HALF_RTE) == 0x7BFF, "half(65504) = 0x7BFF, the largest finite half");
  QT_CHECK(cl_half_from_float(0.1f, CL_HALF_RTE) == 0x2E66 && cl_half_from_float(0.1f, CL_HALF_RTZ) == 0x2E66 &&
           cl_half_from_float(0.1f, CL_HALF_RTN) == 0x2E66, "half(0.1) = 0x2E66 to nearest, towards zero, downwards");
  QT_CHECK(cl_half_from_float(0.1f, CL_HALF_RTP) == 0x2E67, "half(0.1) = 0x2E67 upwards");
  QT_CHECK(cl_half_from_float(1.0f / 3, CL_HALF_RTE) == 0x3555 && cl_half_from_float(1.0f / 3, CL_HALF_RTP) == 0x3556,
           "half(1/3) = 0x3555 to nearest, 0x3556 upwards");
  QT_CHECK(cl_half_from_float(1.0f + 1.0f / 2048, CL_HALF_RTE) == 0x3C00 &&
           cl_half_from_float(1.0f + 1.0f / 2048, CL_HALF_RTP) == 0x3C01, "the tie 1 + 2^-11 rounds to even (0x3C00), upwards to 0x3C01");
  QT_CHECK(cl_half_from_float(1e10f, CL_HALF_RTE) == 0x7C00 && cl_half_from_float(1e10f, CL_HALF_RTP) == 0x7C00,
           "overflow gives +infinity (0x7C00) to nearest and upwards");
  QT_CHECK(cl_half_from_float(1e10f, CL_HALF_RTZ) == 0x7BFF && cl_half_from_float(1e10f, CL_HALF_RTN) == 0x7BFF,
           "overflow gives the largest finite half towards zero and downwards");
  QT_CHECK(cl_half_from_float(5.9604644775390625e-8f, CL_HALF_RTE) == 0x0001, "half(2^-24) = 0x0001, the smallest subnormal");
  QT_CHECK(cl_half_from_double(0.1, CL_HALF_RTE) == 0x2E66 && cl_half_from_double(-65520.0, CL_HALF_RTE) == 0xFC00,
           "from double: 0.1 = 0x2E66, -65520 rounds to -infinity (0xFC00)");
  QT_CHECK(cl_half_to_float(0x3C00) == 1.0f && cl_half_to_float(0xC000) == -2.0f && cl_half_to_float(0xFBFF) == -65504.0f,
           "half -> float: 0x3C00 = 1, 0xC000 = -2, 0xFBFF = -65504");
  QT_CHECK(cl_half_to_float(0x3555) == 0.333251953125f, "half -> float: 0x3555 = 0.333251953125");
  QT_CHECK(cl_half_to_float(0x0001) == 5.9604644775390625e-8f, "half -> float: 0x0001 = 2^-24");
  QT_CHECK(isinf(cl_half_to_float(0x7C00)) && isnan(cl_half_to_float(0x7E00)), "half -> float: 0x7C00 is infinity, 0x7E00 a NaN");

  /* cl_platform.h */
  QT_CHECK(sizeof(cl_char) == 1 && sizeof(cl_short) == 2 && sizeof(cl_int) == 4 && sizeof(cl_long) == 8 &&
           sizeof(cl_half) == 2 && sizeof(cl_float) == 4 && sizeof(cl_double) == 8, "scalar types: 1, 2, 4, 8 bytes; cl_half 2");
  QT_CHECK(sizeof(cl_float4) == 16 && sizeof(cl_int3) == 16 && sizeof(cl_double8) == 64 && sizeof(cl_uchar16) == 16,
           "vector types: cl_float4 16, cl_int3 16 (as int4), cl_double8 64, cl_uchar16 16 bytes");
  QT_CHECK(_Alignof(cl_float4) == 16 && _Alignof(cl_long2) == 16, "cl_float4 and cl_long2 are aligned to 16 bytes");
  { cl_float4 v = { { 1.0f, 2.0f, 3.0f, 4.0f } };
    QT_CHECK(v.s[2] == 3.0f, "vector components are reachable as .s[i]"); }
  QT_CHECK((cl_uint)CL_UINT_MAX == 0xffffffffu && CL_LONG_MIN == (-0x7FFFFFFFFFFFFFFFLL - 1) && CL_FLT_EPSILON == 1.1920928955078125e-7f,
           "limits: CL_UINT_MAX, CL_LONG_MIN, CL_FLT_EPSILON = 2^-23");

  /* cl_version.h, cl.h */
#if defined(CL_VERSION_3_0) && defined(CL_VERSION_1_0)
  QT_CHECK(1, "CL_TARGET_OPENCL_VERSION 300 enables CL_VERSION_1_0 ... CL_VERSION_3_0");
#else
  QT_CHECK(0, "CL_TARGET_OPENCL_VERSION 300 enables CL_VERSION_1_0 ... CL_VERSION_3_0");
#endif
  QT_CHECK(CL_MAKE_VERSION(1, 2, 3) == 4202499u, "CL_MAKE_VERSION(1, 2, 3) = 1<<22 | 2<<12 | 3 = 4202499");
  { cl_version v = CL_MAKE_VERSION(3, 0, 19);
    QT_CHECK(CL_VERSION_MAJOR(v) == 3 && CL_VERSION_MINOR(v) == 0 && CL_VERSION_PATCH(v) == 19, "CL_VERSION_MAJOR/MINOR/PATCH unpack 3.0.19"); }
  QT_CHECK(sizeof(cl_name_version) == 68 && offsetof(cl_name_version, name) == 4 && CL_NAME_VERSION_MAX_NAME_SIZE == 64,
           "cl_name_version: a cl_version and 64 bytes of name");
  QT_CHECK(CL_SUCCESS == 0 && CL_DEVICE_NOT_FOUND == -1 && CL_BUILD_PROGRAM_FAILURE == -11 && CL_INVALID_VALUE == -30 &&
           CL_INVALID_KERNEL_NAME == -46 && CL_PLATFORM_NOT_FOUND_KHR == -1001, "error codes (0, -1, -11, -30, -46, -1001)");
  QT_CHECK(CL_DEVICE_TYPE_CPU == 2 && CL_DEVICE_TYPE_GPU == 4 && CL_DEVICE_TYPE_ALL == 0xFFFFFFFF && CL_MEM_READ_WRITE == 1 &&
           CL_MEM_USE_HOST_PTR == 8 && CL_MEM_COPY_HOST_PTR == 32, "device type and memory flag bits");
  QT_CHECK(CL_PLATFORM_NAME == 0x0902 && CL_DEVICE_NAME == 0x102B && CL_PROGRAM_BUILD_LOG == 0x1183 && CL_PLATFORM_ICD_SUFFIX_KHR == 0x0920,
           "query names (CL_PLATFORM_NAME, CL_DEVICE_NAME, CL_PROGRAM_BUILD_LOG, CL_PLATFORM_ICD_SUFFIX_KHR)");

  /* cl_icd.h: the dispatch table */
  QT_CHECK(sizeof(cl_icd_dispatch) == 150 * sizeof(void *), "cl_icd_dispatch has 150 entries");
  QT_CHECK(SLOT(clGetPlatformIDs) == 0 && SLOT(clGetPlatformInfo) == 1 && SLOT(clCreateBuffer) == 14 &&
           SLOT(clBuildProgram) == 30 && SLOT(clEnqueueNDRangeKernel) == 59, "OpenCL 1.0 slots: 0, 1, 14, 30, 59");
  QT_CHECK(SLOT(clCreateSubBuffer) == 82 && SLOT(clCreateImage) == 96, "OpenCL 1.1 starts at slot 82, clCreateImage (1.2) is 96");
  QT_CHECK(SLOT(clCreateCommandQueueWithProperties) == 123 && SLOT(clCloneKernel) == 137 &&
           SLOT(clSetProgramSpecializationConstant) == 145 && SLOT(clCreateBufferWithProperties) == 146,
           "OpenCL 2.0, 2.1, 2.2 and 3.0 slots: 123, 137, 145, 146");
  QT_END();
}
